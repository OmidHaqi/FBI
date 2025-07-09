# Makefile for ForceBindIP Cross-Platform

CXX = g++
CXXFLAGS = -std=c++11 -fPIC -Wall -Wextra
LDFLAGS = -shared -ldl

# Directories
COMMON_DIR = common
LINUX_DIR = linux
INJECTOR_DIR = injector_crossplatform
TEST_DIR = test_crossplatform
BUILD_DIR = build

# Source files
COMMON_SOURCES = $(COMMON_DIR)/network_manager.cpp
LINUX_SOURCES = $(LINUX_DIR)/linux_hooker.cpp
INJECTOR_SOURCES = $(INJECTOR_DIR)/injector.cpp
TEST_IPV4_SOURCES = $(TEST_DIR)/test_ipv4.cpp
TEST_IPV6_SOURCES = $(TEST_DIR)/test_ipv6.cpp

# Object files
COMMON_OBJECTS = $(COMMON_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
LINUX_OBJECTS = $(LINUX_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
INJECTOR_OBJECTS = $(INJECTOR_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TEST_IPV4_OBJECTS = $(TEST_IPV4_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TEST_IPV6_OBJECTS = $(TEST_IPV6_SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Targets
HOOK_LIB = libforcebindip.so
INJECTOR = forcebindip
TEST_IPV4 = test_ipv4
TEST_IPV6 = test_ipv6

.PHONY: all clean install test deb test-deb test-usage

all: $(HOOK_LIB) $(INJECTOR) $(TEST_IPV4) $(TEST_IPV6)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/$(COMMON_DIR) $(BUILD_DIR)/$(LINUX_DIR) $(BUILD_DIR)/$(INJECTOR_DIR) $(BUILD_DIR)/$(TEST_DIR)

# Hook library
$(HOOK_LIB): libforcebindip_simple.c
	gcc -shared -fPIC -ldl $< -o $@

# Injector
$(INJECTOR): $(BUILD_DIR) $(COMMON_OBJECTS) $(INJECTOR_OBJECTS)
	$(CXX) -o $@ $(COMMON_OBJECTS) $(INJECTOR_OBJECTS)

# Test programs
$(TEST_IPV4): $(BUILD_DIR) $(COMMON_OBJECTS) $(TEST_IPV4_OBJECTS)
	$(CXX) -o $@ $(COMMON_OBJECTS) $(TEST_IPV4_OBJECTS)

$(TEST_IPV6): $(BUILD_DIR) $(COMMON_OBJECTS) $(TEST_IPV6_OBJECTS)
	$(CXX) -o $@ $(COMMON_OBJECTS) $(TEST_IPV6_OBJECTS)

# Compile common objects
$(BUILD_DIR)/$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile Linux-specific objects
$(BUILD_DIR)/$(LINUX_DIR)/%.o: $(LINUX_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile injector objects
$(BUILD_DIR)/$(INJECTOR_DIR)/%.o: $(INJECTOR_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test objects
$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Install
install: all
	sudo cp $(HOOK_LIB) /usr/local/lib/
	sudo cp $(INJECTOR) /usr/local/bin/
	sudo ldconfig

# Test
test: all
	@echo "Testing IPv4 binding..."
	@echo "Run: ./$(INJECTOR) -v -4 127.0.0.1 ./$(TEST_IPV4)"
	@echo ""
	@echo "Testing IPv6 binding..."
	@echo "Run: ./$(INJECTOR) -v -6 ::1 ./$(TEST_IPV6)"
	@echo ""
	@echo "Testing interface binding..."
	@echo "First list interfaces: ./$(INJECTOR) -l"
	@echo "Then run: ./$(INJECTOR) -v -i <interface_name> ./$(TEST_IPV4)"

# Clean
clean:
	rm -rf $(BUILD_DIR) $(HOOK_LIB) $(INJECTOR) $(TEST_IPV4) $(TEST_IPV6)

# Build Debian package
deb: all
	@echo "Building Debian package..."
	./build-deb.sh

# Test Debian package
test-deb: deb
	@echo "Testing Debian package..."
	./test-package.sh

# Test all usage scenarios
test-usage:
	@echo "Testing all ForceBindIP usage scenarios..."
	./test-all-usage.sh

# Help
help:
	@echo "ForceBindIP Cross-Platform Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build everything"
	@echo "  clean    - Remove all build files"
	@echo "  install  - Install to system (requires sudo)"
	@echo "  test     - Show test commands"
	@echo "  deb      - Build Debian package (.deb)"
	@echo "  test-deb  - Build and test Debian package"
	@echo "  test-usage - Test all ForceBindIP usage scenarios"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage Examples:"
	@echo "  make all                          # Build all components"
	@echo "  make deb                          # Build Debian package"
	@echo "  make install                      # Install to system"
	@echo "  ./forcebindip -l                  # List network interfaces"
	@echo "  ./forcebindip -v -4 192.168.1.100 ./test_ipv4  # Bind to specific IPv4"
	@echo "  ./forcebindip -v -i eth0 ./test_ipv4           # Bind to interface"
	@echo ""
	@echo "Package Management:"
	@echo "  sudo dpkg -i forcebindip_1.0.0_amd64.deb      # Install .deb package"
	@echo "  sudo dpkg -r forcebindip                       # Remove package"
