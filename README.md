# ai-torture

can current LLMs make an OS starting from machine code? ||no||

dependencies: nasm, gcc, g++, make, cmake, libvncclient, leptonica, libcurl, nlohmann_json (3.12.0 tested), qemu-system-x86_64

# building

run `make` in the root of the project

# running

ensure that you have set the environment variables `OPENAI_API_KEY`, `OPENAI_API_BASE` (defaults to `https://api.deepseek.com`), `OPENAI_MODEL` (defaults to `deepseek-chat`) and `RFB_PORT` (defaults to 1)

run `make run_qemu` in a separate instance. when a QEMU window shows up, run `make run_jail`.
