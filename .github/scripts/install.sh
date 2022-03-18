#!/bin/bash
# Environment variables used across the build.
PHP_VERSION="8.1.3"

# Fetch PHP source code. This step does not currently validate keys or checksums, as this process
# will eventually transition to using the base `php` Docker images.
export FETCH_DEPS="dpkg-dev software-properties-common apt-transport-https lsb-release ca-certificates curl"
set -xe &&
  sudo apt-get update &&
  sudo apt-get install -y --no-install-recommends ${FETCH_DEPS} &&
  sudo apt-get update

RELEASE_ID=$(lsb_release -si)
# Build PHP library from source.
PHP_PACKAGES="php8.1-dev php8.1-common php8.1-embed php8.1-cli php8.1-opcache libphp8.1-embed php8.1-readline php8.1-opcache php8.1-xml php8.1 php-common"
if [ "${RELEASE_ID}" == "Debian" ]; then
  curl -sSLo /usr/share/keyrings/deb.sury.org-php.gpg https://packages.sury.org/php/apt.gpg &&
    sudo echo "deb [signed-by=/usr/share/keyrings/deb.sury.org-php.gpg] https://packages.sury.org/php/ $(lsb_release -sc) main" >/etc/apt/sources.list.d/php.list
  set -xe &&
		 {
		 echo 'Package: *php*'
		 echo 'Pin: release a=stable-security'
		 echo 'Pin-Priority: -1'
		} >/etc/apt/preferences.d/no-debian-php
elif [ "${RELEASE_ID}" == "Ubuntu" ]; then
  LC_ALL=C.UTF-8 sudo add-apt-repository ppa:ondrej/php -y
fi
sudo apt-get update &&
  export CFLAGS="${PHP_CFLAGS}" CPPFLAGS="${PHP_CPPFLAGS}" LDFLAGS="${PHP_LDFLAGS}"
sudo apt-get install -y --no-install-recommends ${PHP_PACKAGES}
sudo ln -sT "/usr/include/php/$(ls /usr/include/php)" /usr/include/php/phpsrc
export PHPVERSIONID=$(ls /usr/include/php) &&
  export arch="$(dpkg-architecture --query DEB_BUILD_GNU_TYPE)" &&
  export multiarch="$(dpkg-architecture --query DEB_BUILD_MULTIARCH)" &&
  echo 'export PHPVERSIONID=$(ls /usr/include/php)' >>/etc/profile &&
  . /etc/profile
