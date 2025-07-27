# AreaOfInterest
AreaOfInterest는 MMORPG 프로젝트 제작 중 가시거리의 일부 문제를 해결하고자 만든 간단한 프로젝트입니다.

## 목적
### 가시거리 트래픽 스파이크 문제

만약 사람들이 많은 일부 지역에 들어가게된다면 해당 영역에 있는 <br>
플레이어들의 모든 정보를 담은 패킷을 받아야 하는 대역폭 증가와 <br>
이를 화면에 렌더링하는 과정에 부담을 주며 경험을 떨어뜨리게 되었습니다. <br>

그래서 해당 문제를 해결하기 위해 가시거리를 나눠 정보와 주기를 조절하여 해결하고자 하였습니다. <br>

## Vcpkg
### SDL2 : 그래픽 및 입력
### Boost.asio : 네트워크

mkdir build <br>
cd build <br>

cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake <br>
cmake --build . --config Release <br>

## 조작
좌 클릭 : 이동 <br>
우 클릭 : 벽 설치 <br>

## 사용법
귀찮으시겠지만 다음 줄을 찾아 주석처리 해주시면 됩니다... <br>
Entity->mIntervalMoveSync = 0.1f; <br>

## 결과 요약
작은 원은 경로에 대한 추가적인 정보와 0.25초마다 위치를 동기화 하게 됩니다.
큰 원은 위치에 대한 동기화를 1초 주기로 할 수 있게 하였습니다.


## 결과 GIF
### 다수의 가시영역
![다수](https://github.com/user-attachments/assets/a4d91460-59ad-4ad0-85b6-0ac1c43cabf9)

### 한개의 가시영역
![한개](https://github.com/user-attachments/assets/30a8bb9e-cb66-4c0c-8961-55561a047736)

## 결과 사진
<img width="468" height="464" alt="image" src="https://github.com/user-attachments/assets/6162d536-2b76-41bd-baca-098f6969192e" />


## 참고
[이득우의 꼭 배워야하는 게임 알고리즘](https://www.inflearn.com/course/%EA%B2%8C%EC%9E%84-%EC%95%8C%EA%B3%A0%EB%A6%AC%EC%A6%98?srsltid=AfmBOop6dMp3k7lA91OPR5NQBIGTTnWZBma8r3uTrY9XFidST7RZB5sU) - A*를 구현하는데 참고 하였습니다.
