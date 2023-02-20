# Lab 3 學號跑馬燈


## I. Introduction

此次 lab 希望大家學會撰寫driver以及使用GPIO腳位，GPIO對應腳位可參考講義。

請在 Raspberry pi 上撰寫<span style="color:red">“學號跑馬燈”</span>

修改Lab3講義提供的driver程式，可以在 Raspberry pi 上透過 writer 將數字寫到 driver 當中，將數字從 driver 中讀出來，並學會運用GPIO腳位顯示於LED

---

請注意，此次lab使用四顆 LED 燈，因此請採用<span style="color:red">二進制顯示你的學號</span>。

EX : 3 ----> 0011 (請亮1和2的LED)

---

## II. Specification

- **driver (自行撰寫)**
  - 撰寫自行定義的 write function，將 writer 傳送過來的學號數值給存起來。
  - 將學號轉成二進制形式顯示於LED
  
- **writer (自行撰寫)**
  - 每隔一秒，於LED上顯示你的學號數字。


## III. Illustration

- driver
  ```
  insmod lab3_driver.ko
  ```

- writer
  ```
  ./writer <student ID>
  ```


## IV. Demo & Submission

- 請將程式碼以下列的格式擺放與命名，以方便助教評分。
  ```
  <學號>_eos_lab3
  |-- Makefile
  |-- writer.c
  |-- lab3_driver.c
  ```
- 請將上述之資料夾壓縮為單一 zip 檔案，並上傳到 E3 上。
