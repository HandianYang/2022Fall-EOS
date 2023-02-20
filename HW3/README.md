# Hw 3 CDC 疫情管理系統（多人連線版本）


## I. Introduction

本次作業將延續 Hw2 的內容，利用 **Process** 或 **Thread** 將伺服器改成多人連線版本，並且要能正確處理 **Race Condition** 的狀況。


## II. Specification

- 此次作業 **只需** 繳交 **server** 的程式，client 會由 **助教提供** 的 checker 進行測試。
  <span style="color:gray">(但還是建議自己準備 client 方便測試及 debug。)</span>

- 此次疫情管理系統有可以多人連線，有 **兩台** 救護巴士可以載送病人。
  - 系統支援 **多個通報者在線上**，而連線會持續直到中斷連線或回報成功。
  - 只要有一人通報，就會馬上安排救護車護送。
  - 一台救護巴士負責一個區域的確診病患。
  - 救護採 **FCFS** 。按照 server 收到的指令順序，將需要救護的地區。
    <span style="color:gray">(在救護巴士的安排上，哪個救護巴士目前等待的時間較少就安排至該台救護巴士)</span>

---

<span style="color:red">
假設兩台救護巴士都在護送中，在此同時還有人員通報
如果通報的地區與現在正在等待的區域有重複
同一區域人數將合併，而等待秒數依照最先通報的區域為主
</span>

---
  
- client 連線後可執行的指令有
  1. Confirmed case : 系統顯示各區域人數
    進入此狀態後，輸入該區域對應數字則顯示輕症、重症人數
  2. Reporting system : 通報系統
  3. Exit : 離開系統

---

<span style="color:red">
下面的操作可以多個通報人員 “同時” 使用
</span>

---

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
The ambulance is on it's way...

<!-- 依照地區編號等待秒數 -->
<!-- server 等待 8 秒後回傳以下資訊給 client -->
Area 8 | Mild 5 

<!-- client 傳以下資訊給 server -->
Confirmed case | Area 8

<!-- server 回傳以下資訊給 client -->
Area 8 - Severe : 0 | Mild : 5 

<!-- client 傳以下資訊給 server -->
Reporting system | Area 5 | Severe 6 | Area 7 | Mild 1

<!-- server 回傳以下資訊給 client -->
The ambulance is on it's way...

<!-- 如果通報兩個地區以上，等待秒數依照地區編號較大值為主 -->
<!-- server 等待 7 秒後回傳以下資訊給 client -->
Area 5 | Severe 6
Area 7 | Mild 1

<!-- client 傳以下資訊給 server -->
Exit
```

---

### More Example

```xml
<!-- client 1 傳以下資訊給 server -->
Reporting system | Area 8 | Mild 5 

<!-- 經過 4 秒後，client 2 傳以下資訊給 server -->
Reporting system | Area 8 | Severe 2

<!-- 兩個 clients 會同時收到 server 的回覆-->
```

```xml
<!-- client 傳以下資訊給 server -->
Reporting system | Area 8 | Mild 5 | Area 2 | Severe 2 

<!-- 由區域代號小的開始排程，即 Area 2 再來 Area 8 -->
<!-- 經過 8 秒後，sever 回傳以下訊息 -->
Area 2 | Severe 2
Area 8 | Mild 5
```

```xml
<!-- client 傳以下資訊給 server -->
Reporting system | Area 8 | Mild 5 | Area 8 | Severe 2 

<!-- 經過 8 秒後，sever 回傳以下訊息 -->
Area 8 | Severe 2 | Mild 5
```


## III. Illustration

- Server
  ```
  ./hw3 <port>
  ```
  - `<port>` 為 server listen 的 port


- Client
  ```
  ./hw3_checker <ip> <port>
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
  ./hw3_checker <ip> <port>
  ```

- 成功畫面
<img src="https://i.imgur.com/yXhkbv3.jpg">


## IV. Submission

- **請以下列的格式擺放與命名**，以方便助教批改。
  ```
  <學號>_eos_hw3
  |--- Makefile
  |--- hw3.cpp
  |--- ... 
  ```
- 請將上述之資料夾壓縮為單一 zip 檔案，並上傳到 E3 上。
