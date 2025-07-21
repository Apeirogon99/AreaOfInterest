cmake_minimum_required(VERSION 3.20)
project(AreaOfInterest VERSION 1.0.0)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows 설정
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DBOOST_ALL_NO_LIB)
endif()

# 외부 라이브러리 찾기
find_package(SDL2 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# =================================================================
# 공용 정적 라이브러리 생성
# =================================================================
add_library(AreaOfInterestCommon STATIC
    # 헤더 파일들 (Visual Studio 프로젝트에서 보이도록)
    src/Common/AStar/Grid.h
    src/Common/AStar/Node.h
    src/Common/AStar/PathFinding.h

    src/Common/Math/Vector2f.h
    src/Common/Math/Vector2f_NetQuantize.h

    src/Common/Game/Entity.h

    src/Common/Protocol/Message.h
    src/Common/Protocol/Protocol.h

    src/Common/Utils/Time.h
    
    # 소스 파일들
    src/Common/AStar/Grid.cpp
    src/Common/AStar/Node.cpp
    src/Common/AStar/PathFinding.cpp

    src/Common/Math/Vector2f.cpp
    src/Common/Math/Vector2f_NetQuantize.cpp

    src/Common/Game/Entity.cpp

    src/Common/Protocol/Message.cpp
    src/Common/Protocol/Protocol.cpp

    src/Common/Utils/Time.cpp
)

# 공용 라이브러리 설정
target_include_directories(AreaOfInterestCommon PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Visual Studio에서 폴더로 그룹화
set_target_properties(AreaOfInterestCommon PROPERTIES 
    FOLDER "Core Libraries"
    OUTPUT_NAME "AreaOfInterestCommon"
)

# =================================================================
# 클라이언트 실행 파일
# =================================================================
add_executable(AreaOfInterestClient
    # 헤더 파일
    src/Client/Game.h
    src/Client/World.h
    src/Client/Network.h
    src/Client/ClientMessageHandler.h

    # 소스 파일
    src/Client/main.cpp

    src/Client/Game.cpp
    src/Client/World.cpp
    src/Client/Network.cpp
    src/Client/ClientMessageHandler.cpp
)

# 클라이언트 라이브러리 링크
target_link_libraries(AreaOfInterestClient PRIVATE 
    AreaOfInterestCommon      # 공용 라이브러리
    SDL2::SDL2 
    SDL2::SDL2main 
    Boost::system
)

# =================================================================
# 서버 실행 파일  
# =================================================================
add_executable(AreaOfInterestServer
    # 헤더 파일
    src/Server/Game.h
    src/Server/World.h
    src/Server/Network.h
    src/Server/Session.h
    src/Server/Task.h
    src/Server/ServerMessageHandler.h

    # 소스 파일
    src/Server/main.cpp

    src/Server/Game.cpp
    src/Server/World.cpp
    src/Server/Network.cpp
    src/Server/Session.cpp
    src/Server/Task.cpp
    src/Server/ServerMessageHandler.cpp
)

# 서버 라이브러리 링크
target_link_libraries(AreaOfInterestServer PRIVATE 
    AreaOfInterestCommon      # 공용 라이브러리
    Boost::system
)

# =================================================================
# Windows 추가 설정
# =================================================================
if(WIN32)
    target_link_libraries(AreaOfInterestClient PRIVATE ws2_32 wsock32)
    target_link_libraries(AreaOfInterestServer PRIVATE ws2_32 wsock32)
endif()

# =================================================================
# 빌드 후 처리 (선택사항)
# =================================================================
# Release 빌드 시 실행 파일들을 한 곳에 모으기
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(AreaOfInterestClient AreaOfInterestServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
