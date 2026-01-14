SCRIPT_DIR := scripts
BUILD_DIR := build

.PHONY: shaders format build tests clean

check: check_format build tests

shaders:
	@echo "=== Compiling Shaders ==="
	@${SCRIPT_DIR}/shaders.sh

check_format:
	@echo "=== Check Format ==="
	@${SCRIPT_DIR}/check_format.sh

format:
	@echo "=== Format ==="
	@${SCRIPT_DIR}/format.sh

build:
	@echo "=== Build ==="
	@${SCRIPT_DIR}/build.sh

tests:
	@echo "=== Test ==="
	@${SCRIPT_DIR}/test.sh

clean:
	@echo "=== Clean ==="
	@rm -rf ${BUILD_DIR}
