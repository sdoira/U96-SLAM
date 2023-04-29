//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/FPGA.h"
#include "core/Perf.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <termios.h>
#endif

extern Perf perf;

Fpga::Fpga(void)
{
}

Fpga::~Fpga(void)
{
}

int Fpga::registerOpen (void) {
#ifndef _WIN32
	//==================================================================
	// FPGA Register Space (64kB)
	//==================================================================
	// open device file
	if ((fd_dvp = open("/dev/uio0", O_RDWR | O_SYNC)) < 0) {
		LOG_WARN("open failed\n");
		return -1;
	}

	// mapping to virtual address
	address = (long long)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dvp, 0);
	if (address == (long long)MAP_FAILED) {
		LOG_WARN("mmap failed\n");
		close(fd_dvp);
		return -1;
	}

	// register read test
	reg = (struct FPGA_REG *)address;
	LOG_INFO("mmap FPGA register space (FPGA ver:%04X)\n", reg->com.Version);
#endif

	return 0;
}

int Fpga::registerClose (void) {
#ifndef _WIN32
	munmap ((void*)address, 0x10000);
	close(fd_dvp);
#endif
	return 0;
}

int Fpga::memoryOpen (void)
{
#ifndef _WIN32

	//==================================================================
    // rect
	//==================================================================
	// open the physical memory device
	fd_mem_rect = open("/dev/mem", O_RDWR);
	if (fd_mem_rect <= 0) {
		LOG_WARN("failed to open /dev/mem for rect\n");
		exit(1);
	}

	unsigned long from2 = BUF_RECT_A;
	unsigned long num2 = RECT_MAX_SIZE * 2;
	iomap_rect = (volatile unsigned char*)mmap(0, num2, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem_rect, from2);
	if (iomap_rect < 0){
		LOG_WARN("failed to mmap for rect\n");
    	exit(1);
	}

	//==================================================================
    // bm
	//==================================================================
	// open the physical memory device
	fd_mem_bm = open("/dev/mem", O_RDWR);
	if (fd_mem_bm <= 0) {
		LOG_WARN("failed to open /dev/mem for bm\n");
		exit(1);
	}

	unsigned long from3 = BUF_BM_A;
	unsigned long num3 = BM_MAX_SIZE * 2;
	iomap_bm = (volatile unsigned char*)mmap(0, num3, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem_bm, from3);
	if (iomap_bm < 0){
		LOG_WARN("failed to mmap for bm\n");
    	exit(1);
	}

	//==================================================================
    // disp
	//==================================================================
	// open the physical memory device
	fd_mem_disp = open("/dev/mem", O_RDWR);
	if (fd_mem_disp <= 0) {
		LOG_WARN("failed to open /dev/mem for disp\n");
		exit(1);
	}

	unsigned long from4 = BUF_DISP_A;
	unsigned long num4 = DISP_MAX_SIZE * 2;
	iomap_disp = (volatile unsigned char*)mmap(0, num4, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem_disp, from4);
	if (iomap_disp < 0){
		LOG_WARN("failed to mmap for disp\n");
    	exit(1);
	}

	//==================================================================
    // GFTT
	//==================================================================
	// open the physical memory device
	fd_mem_gftt = open("/dev/mem", O_RDWR);
	if (fd_mem_gftt <= 0) {
		LOG_WARN("failed to open /dev/mem for GFTT\n");
		exit(1);
	}

	unsigned long from_gftt = BUF_GFTT_A;
	unsigned long num_gftt = GFTT_MAX_SIZE * 2; // A and B banks
	iomap_gftt = (volatile unsigned char*)mmap(0, num_gftt, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem_gftt, from_gftt);
	if (iomap_gftt < 0){
		LOG_WARN("failed to mmap for GFTT\n");
    	exit(1);
	}
#endif
	return 0;
}

int Fpga::memoryClose (void)
{
#ifndef _WIN32
	unsigned long num2 = RECT_MAX_SIZE * 2;
	munmap ((void*)iomap_rect, num2);
	close (fd_mem_rect);

	unsigned long num3 = BM_MAX_SIZE * 2;
	munmap ((void*)iomap_bm, num3);
	close (fd_mem_bm);

	unsigned long num4 = DISP_MAX_SIZE * 2;
	munmap ((void*)iomap_disp, num4);
	close (fd_mem_disp);

	unsigned long num_gftt = GFTT_MAX_SIZE * 2;
	munmap ((void*)iomap_gftt, num_gftt);
	close (fd_mem_gftt);
#endif
	return 0;
}

void Fpga::uioIrqOn(int uio_fd)
{
#ifndef _WIN32
    unsigned int irq_on = 1;
	write(uio_fd, &irq_on, sizeof(irq_on));
#endif
}

void Fpga::uioIrqOff(int uio_fd)
{
#ifndef _WIN32
    unsigned int irq_on = 0;
    write(uio_fd, &irq_on, sizeof(irq_on));
#endif
}

int Fpga::uioIrqWait(int uio_fd)
{
#ifndef _WIN32
    unsigned int  count = 0;
    return read(uio_fd, &count,  sizeof(count));
#else
	return 0;
#endif
}


//=============================================================================
//! FPGA Read Version
//-----------------------------------------------------------------------------
//! @return FPGA version number
//=============================================================================
int Fpga::readVersion (void) {
#ifdef _WIN32
	return 0;
#else
	return (reg->com.Version);
#endif
}

void Fpga::sendIpcMessage (
	unsigned int Message,
	unsigned int Parameter1,
	unsigned int Parameter2,
	unsigned int Parameter3,
	unsigned int Parameter4
) {
	reg->com.IpcParameter1 = Parameter1;
	reg->com.IpcParameter2 = Parameter2;
	reg->com.IpcParameter3 = Parameter3;
	reg->com.IpcParameter4 = Parameter4;
	reg->com.IpcMessage1 = Message;
}

void Fpga::waitIpcMessage(unsigned int message) {
	while (reg->com.IpcMessage2 != message) {}
	reg->com.IpcMessage2 = IPC_MSG_NONE;
}

void Fpga::waitIpcMessage_Perf(unsigned int message) {
	perf.startTime("frame_wait");
	waitIpcMessage(message);
	perf.stopTime("frame_wait");
}

void Fpga::startRemoteApp (unsigned int mode, unsigned int pattern) {
	sendIpcMessage(
		IPC_MSG1_APP_START,
		mode,
		pattern
	);
}

void Fpga::setRectImage(int bank, cv::Mat imageLeft, cv::Mat imageRight)
{
	// left image start address
	unsigned char *src_left = (unsigned char*)iomap_rect;
	if (bank != 0) {
		src_left += RECT_MAX_SIZE;
	}

	// right image start address
	unsigned char *src_right = src_left + RECT_FRAME_OFFSET;

	memcpy((void*)src_left, imageLeft.data, imageLeft.total());
	memcpy((void*)src_right, imageRight.data, imageRight.total());
}

void Fpga::receiveRectImages(int bank, cv::Mat &matLeft, cv::Mat &matRight)
{
	// left
	unsigned char *src_left = (unsigned char*)iomap_rect;
	if (bank != 0) {
		src_left += RECT_MAX_SIZE;
	}
	cv::Mat leftRaw = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1, src_left);
	cv::Mat left = leftRaw.clone();

	// right
	unsigned char *src_right = src_left + RECT_FRAME_OFFSET;
	cv::Mat rightRaw = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1, src_right);
	cv::Mat right = rightRaw.clone();

	matLeft = left;
	matRight = right;
}

void Fpga::receiveDepthMap(int bank, cv::Mat &matDepth)
{
	short *src_disp = (short*)iomap_disp;
	if (bank != 0) {
		src_disp += (DISP_MAX_SIZE / sizeof(*src_disp));
	}
	cv::Mat depthRaw = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_16SC1, src_disp);
	cv::Mat depth = depthRaw.clone();
	matDepth = depth;
}

void Fpga::receiveEigen(int bank, cv::Mat &matEigen, unsigned short *maxEigen)
{
	unsigned short *src_gftt = (unsigned short*)iomap_gftt;
	if (bank != 0) {
		src_gftt += (GFTT_MAX_SIZE / sizeof(*src_gftt));
	}
	cv::Mat eigRaw = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_16UC1, src_gftt);
	cv::Mat eig = eigRaw.clone();
	matEigen = eig;

	unsigned int max = reg->gftt.Max;
	if (bank == 0) {
		*maxEigen = (unsigned short)(max & 0x0000FFFF);
	} else {
		*maxEigen = (unsigned short)((max & 0xFFFF0000) >> 16);
	}
}

int Fpga::readSwitch(void) {
	// returns the state of the switch
	// 1 if pressed.
	if ((reg->com.GPIO_In & 0x00000002) == 0x00000002) {
		return 1;
	}
	else {
		return 0;
	}
}

void Fpga::receiveData(SensorData &data, APP_SETTING appSetting)
{
	// wait for data ready
	waitIpcMessage_Perf(IPC_MSG2_DATA_READY);
	int activeBank = reg->com.IpcParameter2;

	// rectified stereo images
	if (appSetting.inputType == INPUT_TYPE_SENSOR)
	{
		perf.startTime("receiveRectImages");
		cv::Mat matLeft, matRight;
		receiveRectImages(activeBank, matLeft, matRight);
		data.setStereoImage(matLeft, matRight);
		perf.stopTime("receiveRectImages");
	}

	// dense depth map
	if (appSetting.depthMethod == DEPTH_METHOD_FPGA_BM)
	{
		perf.startTime("receiveDepthMap");
		cv::Mat matDepth;
		receiveDepthMap(activeBank, matDepth);
		data.setImageDepth(matDepth);
		perf.stopTime("receiveDepthMap");
	}

	// dense eigenvalue for GFTT
	if (appSetting.kptsMethod == KPTS_METHOD_FPGA_GFTT)
	{
		perf.startTime("receiveEigen");
		cv::Mat matEigen;
		unsigned short maxEigen;
		receiveEigen(activeBank, matEigen, &maxEigen);
		data.setImageEigen(matEigen);
		data.setMaxEigen(maxEigen);
		perf.stopTime("receiveEigen");
	}
}

int Fpga::isSwitchPressed(void) {
	if ((reg->com.SwitchHold & 0x00000001) == 0x00000001) {
		reg->com.SwitchHold = 0x00000001; // write 1 to clear
		return 1;
	}
	else {
		return 0;
	}
}

void Fpga::ledOn(void) {
	unsigned int tmpi;
	tmpi = reg->com.GPIO_Out;
	tmpi |= 0x00000002;
	reg->com.GPIO_Out = tmpi;
}

void Fpga::ledOff(void) {
	unsigned int tmpi;
	tmpi = reg->com.GPIO_Out;
	tmpi &= ~0x00000002;
	reg->com.GPIO_Out = tmpi;
}

void Fpga::ledBlink(int rate) {
	ledOff();
	reg->com.Blink = (rate & 0x7) << 8; // mikrobus site2
}

unsigned int Fpga::readTimer(void) {
	//return reg->com.Timer;

#ifndef _WIN32
	return reg->com.Timer;
#endif
	return 0;
}

