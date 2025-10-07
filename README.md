# NTUST-DSD-Project2

## 編譯步驟（CMake）

1. 安裝 CMake：

	```bash
	sudo apt update
	sudo apt install cmake
	```

2. 在專案根目錄開啟終端機，執行：

	```bash
	mkdir build
	cd build
	cmake ..
	cmake --build .
	```

	編譯完成後，會在 `build` 資料夾產生可執行檔（如 `main`）。

---

## 使用教學

1. 準備好 PLA 格式的輸入檔案（可參考 `res` 資料夾）。
2. 執行程式：

	```bash
	./main <input.pla> <output.pla>
	```

	範例：

	```bash
	./main ../res/input.pla ../res/output.pla
	```

	執行後，結果會輸出到指定的 output 檔案。

---


## res 資料夾說明

- `input.pla`：基本範例輸入檔
- `input1.pla`、`input2.pla`、`input3.pla`：TA 提供的測試範例
- `custom1.pla`、`custom2.pla`、`custom3.pla`：自行產生的測試檔案

---

## VS Code lldb-mi 高 CPU bug 與臨時修復

在 VS Code 使用 CMake Tools 進行 Debug 時，可能會遇到 lldb-mi 佔用大量 CPU 資源的問題。

### 臨時修復方法
請打開 `.vscode/launch.json`，將 `type` 欄位由 `cppdbg` 改為 `lldb`，如下：

```jsonc
{
	"version": "0.2.0",
	"configurations": [
			{
					//...其他設定
					"type": "lldb", // change this one from cppdbg to lldb
			}
	]
}
```

參考 [GitHub Issue #7240](https://github.com/microsoft/vscode-cpptools/issues/7240)

---

