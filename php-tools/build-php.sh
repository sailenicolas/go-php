#!/bin/bash
# Environment variables used across the build.
export PHP_VERSION="8.1.3"
export PHP_URL="https://secure.php.net/get/php-${PHP_VERSION}.tar.xz/from/this/mirror"
export PHP_BASE_DIR="/tmp/php"
export PHP_SRC_DIR="${PHP_BASE_DIR}/src"
export PHP_BASE_INSTALL="/usr/include/php"
export STATIC=false

# Build variables.
export PHP_LDFLAGS="-Wl,-O1 -Wl,--hash-style=both -pie"
export PHP_CFLAGS="-fstack-protector-strong -fpic -fpie -O2"
export PHP_CPPFLAGS="${PHP_CFLAGS}"

# Fetch PHP source code. This step does not currently validate keys or checksums, as this process
# will eventually transition to using the base `php` Docker images.
export FETCH_DEPS="ca-certificates wget"
set -xe &&
  apt-get update && apt-get install -y --no-install-recommends ${FETCH_DEPS} &&
  mkdir -p ${PHP_BASE_DIR} && cd ${PHP_BASE_DIR}
if [[ ! -f ${PHP_BASE_DIR}/php-${PHP_VERSION}.tar.xz ]]; then
  wget -O php-${PHP_VERSION}.tar.xz ${PHP_URL}
fi

# Build PHP library from source.
export BUILD_DEPS="build-essential file libpcre3-dev dpkg-dev libcurl4-openssl-dev libedit-dev libsqlite3-dev libssl-dev libxml2-dev zlib1g-dev libonig-dev"
set -xe &&
  sudo apt-get update
sudo apt-get install -y --no-install-recommends ${BUILD_DEPS}
export CFLAGS="${PHP_CFLAGS}"
export CPPFLAGS="${PHP_CPPFLAGS}"
export LDFLAGS="${PHP_LDFLAGS}"
export arch="$(dpkg-architecture --query DEB_BUILD_GNU_TYPE)" && multiarch="$(dpkg-architecture --query DEB_BUILD_MULTIARCH)"
[ "x$STATIC" = "xfalse" ] &&
  options="--enable-embed" ||
  options="--enable-embed=static --enable-static"
[ -d "/usr/local/include/curl" ] && [ ! -L "/usr/local/include/curl" ] && ln -sT "/usr/include/$multiarch/curl" /usr/local/include/curl
[ -d ${PHP_BASE_INSTALL} ] && sudo rm -rf ${PHP_BASE_INSTALL}
[ ! -d ${PHP_BASE_INSTALL} ] && sudo mkdir -p ${PHP_BASE_INSTALL}
[ ! -d ${PHP_SRC_DIR} ] && sudo mkdir -p ${PHP_SRC_DIR}
cd ${PHP_SRC_DIR} &&
  tar -xJf ${PHP_BASE_DIR}/php-${PHP_VERSION}.tar.xz -C ${PHP_BASE_INSTALL} --strip-components=1 && cd ${PHP_BASE_INSTALL} &&
  ./configure \
    --prefix=/usr --build="$arch" \
    --with-libdir="lib/$multiarch" \
    --disable-cgi --disable-fpm \
    --enable-opcache \
    --enable-ftp --enable-mbstring \
    --with-curl --with-libedit --with-openssl --with-zlib \
    $options &&
  make -j "$(nproc)"

# Install runtime dependencies for testing, building packages etc, and clean up source.
export RUNTIME_DEPS="build-essential git curl libssl1.1 libpcre3-dev libcurl4-openssl-dev libedit-dev libxml2-dev zlib1g-dev"
set -xe &&
  apt-get update && apt-get install -y --no-install-recommends ${RUNTIME_DEPS} &&
  cd ${PHP_BASE_INSTALL} && make -j "$(nproc)" PHP_SAPI=embed install-sapi install-headers
