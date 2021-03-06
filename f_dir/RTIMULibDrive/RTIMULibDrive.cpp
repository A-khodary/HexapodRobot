#include "RTIMULib.h"
#include "RTMath.h"
#include <math.h>
#include <iostream>
#include <vector>
using namespace std;

//void RTUMULibDrive()
extern "C"{
	extern int moveDirection;
	int sampleCount = 0;
	int sampleRate = 0;

	uint64_t rateTimer;
	uint64_t displayTimer;
	uint64_t now;
	RTIMUSettings *settings;
	RTIMU *imu;

	int stdX = 0;
	int stdY = 0;
	int stdZ = 0;	
	float degree[3];
	vector<float> vecX;
	vector<float> vecY;
	vector<float> vecZ;
	vector<int> vecZint;

	void gyroInit();
	void getDegree(float degree[]);
	RTFLOAT gyroSensor();
	
	void showVector(){
		//int i;
		//for(i=0;i<vecZint.size();i++){
			//printf("%d => %f %f %f\n",i,vecX[i],vecY[i],vecZ[i]);
			//printf("%d => %f\n",i,vecZ[i]);
			//printf("%d => %d\n",i,vecZint[i]);
		//}
	}
	void setStd(){
		getDegree(degree);
		//stdX = degree[0];
		//stdY = degree[1];
		stdZ = degree[2];
		//vecZ.push_back(degree[2]);
		vecZint.push_back((int)degree[2]);
	}

	int cnt = 0;
	void storeVector(){
		int curr;
		//vecX.push_back(degree[0]);
		//vecY.push_back(degree[1]);
		//printf("Z -> %d\n",(int)degree[2]);
		getDegree(degree);
		curr = (int)degree[2];
		
		if(abs(curr - vecZint.back()) <= 40) {
			vecZint.push_back(curr);
			cnt++;
			if(cnt == 50) {
				//printf("GyroZ => %d\n",curr);
				cnt = 0;
			}
		}
		//printf("GyroZ => %f\n",degree[2]);
   }
	void tmpStore(){
		//vecX.push_back(0);
		//vecY.push_back(0);
		//vecZ.push_back(0);
	}
	
	//앞으로 직진 -> 한싸이클 하고 벡터저장
	//changedir 방향 변경 -> 함수한번 부를때마다 벡터저장한다고 치면
	//장애물감지 안돼서 직진하기 전에 analyVector?
	const int rangeMax = 28;
	const int rangeMin = -28;
	int cnt2 = 0;
	int leftcnt = 0;
	int rightcnt = 0;
	void analyVector() {
		//벡터내의 모든 값들을 이전값과 비교해서 뺀값을 구해서 방향결정?
		//다시 원위치와서 차이값이 0일때 벡터비우기
		//리턴값 0:직진, 1:왼쪽회전, 2:오른쪽회전
		int i;
		int diff=0;
		for (i = vecZint.size() - 1; i > 0; i--) {
			diff += vecZint[i] - vecZint[i - 1];
		}
		cnt2++;
		if(cnt2 == 50) {
			printf("Diff => %d\n",diff);
			cnt2 = 0;
		}	
		if (diff >= rangeMax+9) {
			moveDirection = 1;
			leftcnt++;
			if(leftcnt%50==0)
				printf("Want to go left : %d\n",diff);
		}else if (diff <= rangeMin-9) {
			moveDirection = 2;
			rightcnt++;
			if(rightcnt%50==0)
				printf("Want to go Right : %d\n",diff);
		}else if (diff < rangeMax && diff > rangeMin) {
			//printf("Want to go Forward : %d\n",diff);
			//벡터비우기 : 초기값 제외
			vecZint.clear();
			vecZint.push_back(stdZ);
			moveDirection = 0;
		}
		return;
	}

	void gyroInit(){
		//  Using RTIMULib here allows it to use the .ini file generated by RTIMULibDemo.
		//  Or, you can create the .ini in some other directory by using:
		//      RTIMUSettings *settings = new RTIMUSettings("<directory path>", "RTIMULib");
		//  where <directory path> is the path to where the .ini file is to be loaded/saved
		settings = new RTIMUSettings("RTIMULib");
		imu = RTIMU::createIMU(settings);

		if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
			printf("No IMU found\n");
			exit(1);
		}
		//  This is an opportunity to manually override any settings before the call IMUInit
		//  set up IMU
		imu->IMUInit();
		//  this is a convenient place to change fusion parameters
		imu->setSlerpPower(0.02);
		imu->setGyroEnable(true);
		imu->setAccelEnable(true);
		imu->setCompassEnable(true);
		//  set up for rate timer
		rateTimer = displayTimer = RTMath::currentUSecsSinceEpoch();
		//  now just process data
	}

	void getDegree(float degree[]){
		//  now just process data
		while (1) {
			//  poll at the rate recommended by the IMU
			while (imu->IMURead()) {
				RTIMU_DATA imuData = imu->getIMUData();
				degree[0] = imuData.fusionPose.x() * RTMATH_RAD_TO_DEGREE;
				degree[1] = imuData.fusionPose.y() * RTMATH_RAD_TO_DEGREE;
				degree[2] = imuData.fusionPose.z() * RTMATH_RAD_TO_DEGREE;
				//printf("\n");
				//printf("***degree test*** x = %f, y = %f, z = %f\n",degreeX,degreeY,degreeZ);
				return ;
			}
		}
	}
	RTFLOAT gyroSensor()
	{
		float degreeX, degreeY, degreeZ;
		while (1) {
			//  poll at the rate recommended by the IMU
			usleep(imu->IMUGetPollInterval() * 1000);

			while (imu->IMURead()) {
				RTIMU_DATA imuData = imu->getIMUData();
				sampleCount++;
				now = RTMath::currentUSecsSinceEpoch();

				//  display 10 times per second

				if ((now - displayTimer) > 100000) {
					degreeX = imuData.fusionPose.x() * RTMATH_RAD_TO_DEGREE;
					degreeY = imuData.fusionPose.y() * RTMATH_RAD_TO_DEGREE;
					degreeZ = imuData.fusionPose.z() * RTMATH_RAD_TO_DEGREE;
					printf("***degree test*** x = %f, y = %f, z = %f\n",degreeX,degreeY,degreeZ);
					displayTimer = now;
				}

				//  update rate every second
				if ((now - rateTimer) > 1000000) {
					sampleRate = sampleCount;
					sampleCount = 0;
					rateTimer = now;
				}
			}
		}
	}
}
