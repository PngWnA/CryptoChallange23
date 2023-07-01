# 분석 진행 환경
* Macbook M1 Pro

# 0. 주어진 파일 압축 해제
* Tool(s): p7zip

* Command: `7z e ./5번문제_데이터_수정본.7z`

* From: 5번문제_데이터_수정본.7z

* To: 1.png, 2.bmp, 3.bmp, 4.png

# 1. 1.png 조사
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

* Command: `7z e ./1.a.7.7z`

> Note: 명령어 실행 후 비밀번호(#2023$CRYPT@NALY$IS2023#) 입력 필요

> Note: partition.vhd는 커서 제외하고 커밋함

* From: 1.a.7.7z

* To: partition.vhd, P.txt

## 1.5 partition.vhd 파일 조사

* Tool(s): HxD, FTK Imager

> Note: [FAT32 파티션 복구](https://s0ng.tistory.com/entry/%EB%94%94%EC%8A%A4%ED%81%AC-%ED%8F%AC%EB%A0%8C%EC%8B%9D-FAT32-%ED%8C%8C%ED%8B%B0%EC%85%98-%EB%B3%B5%EA%B5%AC)

* Copy sector 134 to sector 128

* From: partition.vhd

* To: enc_blueprint

# 2. 2.bmp 조사
## 2.1. 기본적인 파일 조사
* Tool(s): stegsolve

* Note: 분석기능 제대로 작동 안함

## 2.2. 기본적인 파일 조사 (2)
* Tool(s): binwalk

* Command: `binwalk -eM ./2.bmp`

* Note: 뒤쪽에 7z 데이터 붙어있는 것 확인

* Note: 제대로 추출되지는 않음

## 2.3. 7z 파일 카빙
* Tool(s): python

* Command: 아래 첨부

```py
a = open("2.bmp", "rb").read()
open("2.out.7z", "wb").write(a[0x2c146:])
```

* Note: 0x2C146에서부터 끝까지 추출

* From: 2.bmp

* To: 2.out.7z

## 2.4. 7z 파일 추출
* Tool(s): p7zip

* Command: `7z e ./2.out.7z`

* Note: 추출결과 첨부


```
   Date      Time    Attr         Size   Compressed  Name
------------------- ----- ------------ ------------  ------------------------
2023-03-16 23:17:22 ....A      7547798     16951552  enc_3.bmp
2023-03-16 23:17:23 ....A      8492123               enc_4.png
2023-03-16 23:17:24 ....A      7927887               enc_5.png
2023-03-16 23:42:28 ....A         1225               README_README.txt
------------------- ----- ------------ ------------  ------------------------
2023-03-16 23:42:28           23969033     16951552  4 files
```

* From: 2.out.7z

* To: enc_3.bmp, enc_4.png, enc_5.png, README_README.txt

## 2.5. 파일 복호화 코드 작성 (구현중)
* Tool(s): python

* Command: `python decryptor.py $FILENAME`

* Note: README_README.txt 내용 참고하여 복호화코드 작성