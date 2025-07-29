# AreaOfInterest
AreaOfInterest는 MMORPG 프로젝트 제작 중 가시거리의 일부 문제를 해결하고자 만든 간단한 프로젝트입니다.

## 목적
### 플레이어 시야밖 불필요한 정보와 동기

가시거리의 특성상 플레이어 화면에 없다가 갑작스럽게 나타나게 된다면 <br>
유저는 어색함을 느끼게 되어 화면보다는 좀 더 큰 범위에 영역을 설정합니다. <br>

문제는 자연스러움을 위해 화면에 보이지 않는 플레이어들의 정보를 모두 동기화 <br>
하는 것은 또 다시 불필요한 트래픽이 발생하고 있어 해결하고자 하였습니다. <br>

모든 정보가 플레이어 시야 밖에서도 동일하다는 것이 문제였습니다. <br>
때문에 플레이어 시야를 기준으로 가시영역을 나누었습니다. <br>

영역에 따라 정보와 동기화 주기를 다르게 설정하였습니다. <br>
큰 영역은 플레이어와 존재와 필수 정보(위치)와 느린 동기화 주기를 <br>
작은 영역은 모든 정보와 빠른 동기화로 트래픽을 감소 시켰습니다. <br>

## 테스트 환경
<img width="468" height="464" alt="image" src="https://github.com/user-attachments/assets/6162d536-2b76-41bd-baca-098f6969192e" />

## 결과 요약
트래픽 62% 절약하면서도 품질을 유지하였습니다.

## 결과 GIF
### 이중 가시영역
![이중 가시영역](https://github.com/user-attachments/assets/a4d91460-59ad-4ad0-85b6-0ac1c43cabf9)

### 단일 가시영역
![단일 가시영역](https://github.com/user-attachments/assets/30a8bb9e-cb66-4c0c-8961-55561a047736)

## Vcpkg
### SDL2 : 그래픽 및 입력
### Boost.asio : 네트워크

mkdir build <br>
cd build <br>

cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake <br>
cmake --build . --config Release <br>

## 조작
좌 클릭 : 이동 <br>

## 사용법
귀찮으시겠지만 다음 줄을 찾아 수정해주시면 됩니다... <br>
#define USE_AOI 1 = 이중 가시영역 <br>
#define USE_AOI 1 = 단일 가시영역 <br>

## 참고
[이득우의 꼭 배워야하는 게임 알고리즘](https://www.inflearn.com/course/%EA%B2%8C%EC%9E%84-%EC%95%8C%EA%B3%A0%EB%A6%AC%EC%A6%98?srsltid=AfmBOop6dMp3k7lA91OPR5NQBIGTTnWZBma8r3uTrY9XFidST7RZB5sU) - A*를 구현하는데 참고 하였습니다.
