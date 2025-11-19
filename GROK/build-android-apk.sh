#!/bin/bash
# TBOS Android APK Builder
# 🕉️ Swamiye Saranam Aiyappa 🕉️

set -e

VERSION="1.0"
VERSION_CODE="1"
APK_NAME="TBOS-Detector-Android-v${VERSION}.apk"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  Building TBOS Android APK                              ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="${SCRIPT_DIR}/app/android"
DIST_DIR="${SCRIPT_DIR}/dist"

mkdir -p "${DIST_DIR}"

echo "[1/5] Checking dependencies..."
if ! command -v gradle &> /dev/null; then
    echo "Installing Gradle..."
    if command -v apt &> /dev/null; then
        sudo apt install -y gradle
    else
        echo "Please install Gradle manually: https://gradle.org/install/"
        exit 1
    fi
fi
echo "✅ Dependencies ready"

echo "[2/5] Preparing Android project..."
cd "${ANDROID_DIR}"

# Update version in build.gradle if it exists
if [ -f "app/build.gradle" ]; then
    sed -i "s/versionCode [0-9]*/versionCode ${VERSION_CODE}/" app/build.gradle
    sed -i "s/versionName \".*\"/versionName \"${VERSION}\"/" app/build.gradle
fi

echo "✅ Project prepared"

echo "[3/5] Building APK..."

# Clean previous builds
./gradlew clean

# Build release APK
./gradlew assembleRelease

echo "✅ APK built"

echo "[4/5] Signing APK..."

# Check if keystore exists
KEYSTORE="${SCRIPT_DIR}/tbos-release-key.keystore"
if [ ! -f "${KEYSTORE}" ]; then
    echo "Creating release keystore..."
    keytool -genkey -v \
        -keystore "${KEYSTORE}" \
        -alias tbos-key \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -storepass tbospass \
        -keypass tbospass \
        -dname "CN=TBOS Project, OU=Development, O=TBOS, L=Sacred, ST=Digital, C=IN"
    echo "✅ Keystore created"
fi

# Sign APK
UNSIGNED_APK="app/build/outputs/apk/release/app-release-unsigned.apk"
SIGNED_APK="${DIST_DIR}/${APK_NAME}"

if [ -f "${UNSIGNED_APK}" ]; then
    jarsigner -verbose \
        -sigalg SHA256withRSA \
        -digestalg SHA-256 \
        -keystore "${KEYSTORE}" \
        -storepass tbospass \
        -keypass tbospass \
        "${UNSIGNED_APK}" \
        tbos-key

    # Align APK
    if command -v zipalign &> /dev/null; then
        zipalign -v 4 "${UNSIGNED_APK}" "${SIGNED_APK}"
    else
        cp "${UNSIGNED_APK}" "${SIGNED_APK}"
        echo "⚠️  zipalign not found - APK not aligned (still works)"
    fi

    echo "✅ APK signed"
else
    echo "❌ Unsigned APK not found!"
    exit 1
fi

echo "[5/5] Verifying APK..."
if [ -f "${SIGNED_APK}" ]; then
    echo "✅ APK verified"
else
    echo "❌ APK creation failed!"
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  ✅ BUILD COMPLETE                                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "APK: dist/${APK_NAME}"
echo "Size: $(du -h "${SIGNED_APK}" | cut -f1)"
echo ""
echo "To install on Android device:"
echo "  1. Enable 'Unknown Sources' in Settings"
echo "  2. Copy APK to device"
echo "  3. Tap to install"
echo ""
echo "Or via ADB:"
echo "  adb install dist/${APK_NAME}"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
