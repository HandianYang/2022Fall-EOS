# Hw 2 CDC 疫情管理系統（連線版本）


## I. Introduction

本次作業將延續 Hw 1 的內容，將原來單機版本的疫情管理系統利用 socket server 修改成可連線版本。


## II. Specification

- 此次作業 **只需** 繳交 **server** 的程式，client 會由 **助教提供** 的 checker 進行測試。
  <span style="color:gray">(但還是建議自己準備 client 方便測試及 debug。)</span>
- 此疫情管理系統僅有一位人員等待確診情況回報，因此每次僅能服務一名通報者。
  - 當一位通報者連上系統，連線會持續直到中斷連線或回報成功。
  - 當一位通報者正在連線時，不會有第二位通報者連線。
  - 不同次的通報不需紀錄確診人數及症狀。
  
- client 連線後可執行的指令有
  1. Confirmed case : 系統顯示各區域人數
    進入此狀態後，輸入該區域對應數字則顯示輕症、重症人數
  2. Reporting system : 通報系統
  3. Exit : 離開系統

```xml
<!-- server 已開啟 -->
<!-- client 連線 -->

<!-- client 開啟通報查詢list -->
list

<!-- server 接收到 client 的連線傳以下目錄 -->
1. Confirmed case
2. Reporting system
3. Exit

<!-- client 可選擇以上三個選項 -->

<!-- client 傳以下資訊給 server -->
Confirmed case

<!-- server 回傳以下資訊給 client -->
0 : 0
1 : 0
2 : 0
3 : 0
4 : 0
5 : 0
6 : 0
7 : 0
8 : 0

<!-- client 傳以下資訊給 server -->
Reporting system | Area 8 | Mild 5 

<!-- server 回傳以下資訊給 client -->
Please wait a few seconds...

<!-- 依照地區編號等待秒數 -->
<!-- server 等待 8 秒後回傳以下資訊給 client -->
Area 8 | Mild 5 

<!-- client 傳以下資訊給 server -->
Confirmed case | Area 8

<!-- server 回傳以下資訊給 client -->
Area 8 - Mild : 5 | Severe : 0 

<!-- client 傳以下資訊給 server -->
Reporting system | Area 5 | Severe 6 | Area 7 | Mild 1

<!-- server 回傳以下資訊給 client -->
Please wait a few seconds...

<!-- 如果通報兩個地區以上，等待秒數依照地區編號較大值為主 -->
<!-- server 等待 7 秒後回傳以下資訊給 client -->
Area 5 | Severe 6
Area 7 | Mild 1

<!-- client 傳以下資訊給 server -->
Exit
```

---

< 補充說明 > 

- client 傳送 :
  ```
  Area 5 | Mild 6 | Area 5 | Severe 1
  ```

- server 回傳 :
  ```
  Area 5 | Mild 6
  Area 5 | Severe 1
  ```

---


## III. Illustration

- Server
  ```
  ./hw2 <port>
  ```
  - `<port>` 為 server listen 的 port


- Client
  ```
  ./hw2_checker <ip> <port>
  ```

- Checker
  - 一個簡單測試的測試程式，他會連續打入很多指令
  - Checker的運作流程為:
    - 1.和server建立連線
    - 2.傳輸資料給server
    - 3.等待server回傳資料
    - 重複2.與3.直到該用戶完成所有指令後斷線並返回1.
    - 重複以上流程直到測試資料全部輸入完成
  - 請務必檢查參數的數量是否正確。
  ```
  ./hw2_checker <ip> <port>
  ```
  - 下載網址
  - 成功執行範例(約需35~40秒)
    - 若超過20秒沒回應，則應該是checker在等待server傳訊息過來，但server沒傳。請檢查是否有command沒傳送到。

- 成功畫面
<img src="https://i.imgur.com/3aorNfS.png">


## IV. Submission

- **請以下列的格式擺放與命名**，以方便助教批改。
  ```
  <學號>_eos_hw2
  |--- Makefile
  |--- hw2.cpp
  |--- ... 
  ```
- 請將上述之資料夾壓縮為單一 zip 檔案，並上傳到 E3 上。
