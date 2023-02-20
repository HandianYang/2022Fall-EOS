# Hw 1 CDC 疫情管理系統（單機版本）


## I. Introduction

近年來疫情肆虐全球，就連有 **“人間天堂”** 之稱的 **艾尼祿尼亞島** 也不敵疫情攻擊，島上的九個區域紛紛有人確診。為了能夠確實掌握疫情動向，因此艾尼祿尼亞島政府特別建立一系統去監控確診發生情況。

請在 Raspberry Pi 上撰寫一隻CDC 疫情管理系統，實現通報與查詢之功能。


## II. Specification

- 疫情管理系統
  - 此系統提供通報人員上報疫情狀況，可以在對應的區域通報確診的人數，總共有 9 個區域。
  - 進入系統選單後，選單會因通報人員的需求而改變狀態
    1. Confirmed case : 系統顯示各區域人數
        進入此狀態後，輸入該區域對應數字則顯示輕症、重症人數
    2. Reporting system : 通報系統
    3. Exit : 離開系統
  
- I / O 與 顯示
  - Terminal
    - 輸入及顯示用
  - 7-Segment
    - 顯示總確診人數 或 特定區域確診人數
    - 進入Confirmed case 時顯示 **艾尼祿尼亞島** 總確診人數
    - 查詢特定區域確診通報人數時，顯示目前該區域的確診人數
    - 通報完成時顯示目前確診總人數(含新增人數)
  - LED
    - 排列 九顆 LED 燈，每一顆LED代表不同的地區。
    - 當此區域有確診患者時(不論人數多寡)，對應的LED將會亮燈，在整個程式執行過程中皆不會滅燈 (除查詢特定區域時)。
    - 若查看特定區域的 **確診輕重症資訊** 時，此區域的 LED 燈將會以每隔 0.5 秒持續閃爍 3 秒，閃爍完畢後 **維持亮著**。


## III. Illustration

- Terminal
  ```
  ./hw1
  ```

- Terminal
  ```xml
  <!-- Program menu -->
  1. Confirmed case
  2. Reporting system
  3. Exit

  <!-- Press 1 -->
  <!-- 顯示各區域的確診人數 -->
  <!-- LED 顯示有確診的地區，7-seg 顯示總確診人數 -->
  0 : 0
  1 : 0
  2 : 0
  3 : 0
  4 : 0
  5 : 0
  6 : 0
  7 : 0
  8 : 0 

  <!-- Press 7 -->
  <!-- 顯示該區域輕重症詳細資訊 -->
  <!-- 此區域的 LED 燈將以每隔 0.5 秒持續閃爍 3 秒，閃爍完畢後維持亮著，7-seg 顯示此地區確診人數 -->
  Mild : 0
  Severe : 0  

  <!-- 按任意鍵跳回上一層 -->
  <!-- LED 燈顯示有確診的地區，7-seg 顯示總確診人數 -->
  0 : 0
  1 : 0
  2 : 0
  3 : 0
  4 : 0
  5 : 0
  6 : 0
  7 : 0
  8 : 0

  <!-- 按 'q' 跳回上一層 -->
  1. Confirmed case
  2. Reporting system
  3. Exit


  <!-- Press 2 -->
  Area (0~8) : 

  <!-- Press 8 -->
  Area (0~8) : 8
  Mild or Severe ('m' or 's') : 

  <!-- Press 'm' -->
  Area : 8
  Mild or Severe ('m' or 's') : m
  The number of confirmed case : 

  <!-- Press 1 -->
  Area : 8
  Mild or Severe : m
  The number of confirmed case : 1

  <!-- 對應區域的 LED 要亮，7-seg 顯示目前確診總人數(含已新增) -->
  <!-- 假如人數超過個位數，7-seg 閃爍交替顯示 -->

  <!--'e' : 退回上一層；'c' : 繼續通報 -->
  <!-- Press 'e' -->
  1. Confirmed case
  2. Reporting system
  3. Exit

  <!-- Press 1 -->
  <!-- 顯示各區域的確診人數 -->
  0 : 0
  1 : 0
  2 : 0
  3 : 0
  4 : 0
  5 : 0
  6 : 0
  7 : 0
  8 : 1

  <!-- Press 8 -->
  <!-- 顯示該區域輕重症詳細資訊 -->
  <!-- 此區域的 LED 燈將以每隔 0.5 秒持續閃爍 3 秒，閃爍完畢後維持亮著，7-seg 顯示此地區確診人數 -->
  <!-- 假如人數超過個位數，7-seg 閃爍交替顯示 -->
  Mild : 1
  Severe : 0 

  <!-- 按任意鍵跳回上一層 -->
  0 : 0
  1 : 0
  2 : 0
  3 : 0
  4 : 0
  5 : 0
  6 : 0
  7 : 0
  8 : 1

  <!-- 按 'q' 跳回上一層 -->
  1. Confirmed case
  2. Reporting system
  3. Exit

  <!-- 按 3 退出 -->
  <!-- LED 燈 及 7-seg 全暗 -->
  ```


## IV. Note

- 本次的作業要能透過 make 做編譯的動作，因此請各位在專案資料夾下建立自己的 Makefile。
- 本次作業請用 c 撰寫。
- 完成後請在時間內上傳 e3, 並在課堂上找助教 demo。


## V. Submission

- 請將程式碼以下列的格式擺放與命名，以方便助教批改。
  ```
  <學號>_eos_hw1
  |--- Makefile
  |--- hw1.c    // main function
  |--- ...      // Other source files
  ```
- 請將上述之資料夾壓縮為單一 zip 檔案，並上傳到 E3 上。
