#!/bin/bash
# Environment variables used across the build.
export PHP_VERSION="8.1.3"
export PHP_IV="$(word 1,$(subst ., ,$(PHP_VERSION))).$(word 2,$(subst ., ,$(PHP_VERSION)))"
# Build variables.
export PHP_LDFLAGS="-Wl,-O1 -Wl,--hash-style=both -pie"
export PHP_CFLAGS="-fstack-protector-strong -fpic -fpie -O2"
export PHP_CPPFLAGS="${PHP_CFLAGS}"

# Fetch PHP source code. This step does not currently validate keys or checksums, as this process
# will eventually transition to using the base `php` Docker images.
export FETCH_DEPS="software-properties-common apt-transport-https lsb-release ca-certificates curl"
set -xe && \
    apt-get update && \
    apt-get install -y --no-install-recommends ${FETCH_DEPS} && \
    apt-get update

# Build PHP library from source.
export BUILD_DEPS="dpkg-dev ${PHP_VI}-dev ${PHP_VI}-common ${PHP_VI}-embed ${PHP_VI}-cli ${PHP_VI}-opcache lib${PHP_VI}-embed ${PHP_VI}-readline ${PHP_VI}-opcache ${PHP_VI}-xml ${PHP_VI} php-common lib${PHP_VI}-embed-dbgsym"

curl -vksSLo /usr/share/keyrings/deb.sury.org-php.gpg https://packages.sury.org/php/apt.gpg &&\
    echo "deb [signed-by=/usr/share/keyrings/deb.sury.org-php.gpg] https://packages.sury.org/php/ $(lsb_release -sc) main" > /etc/apt/sources.list.d/php.list && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y && \
    export CFLAGS="${PHP_CFLAGS}" CPPFLAGS="${PHP_CPPFLAGS}" LDFLAGS="${PHP_LDFLAGS}"; \
    apt-get install -y --no-install-recommends ${BUILD_DEPS};

export PHPVERSIONID=$(ls /usr/include/php) && \
    export arch="$(dpkg-architecture --query DEB_BUILD_GNU_TYPE)" &&\
    export multiarch="$(dpkg-architecture --query DEB_BUILD_MULTIARCH)" && \
    echo 'export PHPVERSIONID=$(ls /usr/include/php)' >> /etc/profile && \
	. /etc/profile;
