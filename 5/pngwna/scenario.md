# 분석 진행 환경
* Macbook M1 Pro

# 0. 주어진 파일 압축 해제
* Tool(s): p7zip

* Command: `7z e ./5번문제_데이터_수정본.7z`

* From: 5번문제_데이터_수정본.7z

* To: 1.png, 2.bmp, 3.bmp, 4.png

# 1. 1.bmp 조사
## 1.1. alpha 7채널, red 0채널에서 각각 비트 뽑아서 파일로 저장
* Tool(s): stegsolve

* From: 1.bmp

* To: 1.a.7.7z, 1.r.0.zip

## 1.2. 1.r.0.zip 파일 압축 해제
* Tool(s): unzip

* Command: ./1.r.0.zip

* From: 1.r.0.zip

* To: .P@SSW0rd

## 1.3. .P@SSW0rd 파일 조사
* Filetype: png

* Information: PASSWORD IS #2023$CRYPT@NALY$IS2023#

## 1.4. 1.3에서 추출한 비밀번호 활용하여 1.a.7.7z 파일 압축 해제
* Tool(s): p7zip

* Command: 7z e ./1.a.7.7z

> Note: 명령어 실행 후 비밀번호(#2023$CRYPT@NALY$IS2023#) 입력 필요

> Note: partition.vhd는 커서 제외하고 커밋함

* From: 1.a.7.7z

* To: partition.vhd, P.txt