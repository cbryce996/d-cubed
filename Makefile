SCRIPT_DIR := scripts
BUILD_DIR := build

.PHONY: format build test clean

check: check_format build test

check_format:
	@echo "=== Check Format ==="
	@${SCRIPT_DIR}/check_format.sh

format:
	@echo "=== Format ==="
	@${SCRIPT_DIR}/format.sh

build:
	@echo "=== Build ==="
	@${SCRIPT_DIR}/build.sh

test:
	@echo "=== Test ==="
	@${SCRIPT_DIR}/test.sh

clean:
	@echo "=== Clean ==="
	@rm -rf ${BUILD_DIR}
