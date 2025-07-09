#!/bin/bash

# ForceBindIP Debian Package Builder
# This script builds a .deb package for ForceBindIP

set -e

echo "=== ForceBindIP Debian Package Builder ==="
echo ""

# Check if we're in the right directory
if [ ! -f "libforcebindip_simple.c" ]; then
    echo "❌ Error: Please run this script from the FBI project root directory"
    exit 1
fi

# Build the project first
echo "🔨 Building ForceBindIP..."
make clean
make

# Check if build was successful
if [ ! -f "forcebindip" ] || [ ! -f "libforcebindip.so" ]; then
    echo "❌ Error: Build failed. Missing executables."
    exit 1
fi

echo "✅ Build completed successfully"

# Create package directory structure
PACKAGE_NAME="forcebindip"
VERSION="1.0.0"
ARCH="amd64"
PACKAGE_DIR="${PACKAGE_NAME}_${VERSION}_${ARCH}"

echo "📦 Creating package structure..."
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"

# Create directory structure
mkdir -p "$PACKAGE_DIR/usr/local/bin"
mkdir -p "$PACKAGE_DIR/usr/local/lib"
mkdir -p "$PACKAGE_DIR/usr/local/share/man/man1"
mkdir -p "$PACKAGE_DIR/usr/local/share/doc/forcebindip"
mkdir -p "$PACKAGE_DIR/DEBIAN"

# Copy executables
echo "📋 Copying files..."
cp forcebindip "$PACKAGE_DIR/usr/local/bin/"
cp libforcebindip.so "$PACKAGE_DIR/usr/local/lib/"
cp test_ipv4 "$PACKAGE_DIR/usr/local/bin/"
cp test_ipv6 "$PACKAGE_DIR/usr/local/bin/"

# Copy documentation
cp README.md "$PACKAGE_DIR/usr/local/share/doc/forcebindip/"
cp README_FA.md "$PACKAGE_DIR/usr/local/share/doc/forcebindip/"
cp debian/copyright "$PACKAGE_DIR/usr/local/share/doc/forcebindip/"

# Copy and compress man page
cp debian/forcebindip.1 "$PACKAGE_DIR/usr/local/share/man/man1/"
gzip "$PACKAGE_DIR/usr/local/share/man/man1/forcebindip.1"

# Copy DEBIAN control files
cp debian/control "$PACKAGE_DIR/DEBIAN/"
cp debian/postinst "$PACKAGE_DIR/DEBIAN/"
cp debian/prerm "$PACKAGE_DIR/DEBIAN/"
cp debian/changelog "$PACKAGE_DIR/usr/local/share/doc/forcebindip/"

# Set permissions
echo "🔒 Setting permissions..."
chmod 755 "$PACKAGE_DIR/DEBIAN/postinst"
chmod 755 "$PACKAGE_DIR/DEBIAN/prerm"
chmod 755 "$PACKAGE_DIR/usr/local/bin/forcebindip"
chmod 755 "$PACKAGE_DIR/usr/local/bin/test_ipv4"
chmod 755 "$PACKAGE_DIR/usr/local/bin/test_ipv6"
chmod 644 "$PACKAGE_DIR/usr/local/lib/libforcebindip.so"
chmod 644 "$PACKAGE_DIR/usr/local/share/man/man1/forcebindip.1.gz"
chmod 644 "$PACKAGE_DIR/usr/local/share/doc/forcebindip/"*

# Calculate installed size (in KB)
INSTALLED_SIZE=$(du -sk "$PACKAGE_DIR" | cut -f1)
echo "Installed-Size: $INSTALLED_SIZE" >> "$PACKAGE_DIR/DEBIAN/control"

# Build the package
echo "🔧 Building .deb package..."
dpkg-deb --build "$PACKAGE_DIR"

# Check if package was created
if [ -f "${PACKAGE_DIR}.deb" ]; then
    echo ""
    echo "✅ Package created successfully: ${PACKAGE_DIR}.deb"
    echo ""
    echo "📊 Package information:"
    dpkg-deb --info "${PACKAGE_DIR}.deb"
    echo ""
    echo "📋 Package contents:"
    dpkg-deb --contents "${PACKAGE_DIR}.deb"
    echo ""
    echo "🚀 To install the package:"
    echo "   sudo dpkg -i ${PACKAGE_DIR}.deb"
    echo ""
    echo "🗑️  To remove the package:"
    echo "   sudo dpkg -r forcebindip"
    echo ""
    echo "🔍 To verify installation:"
    echo "   forcebindip --help"
    echo "   forcebindip -l"
else
    echo "❌ Error: Failed to create package"
    exit 1
fi

# Cleanup
echo "🧹 Cleaning up temporary files..."
rm -rf "$PACKAGE_DIR"

echo "🎉 Done!"
