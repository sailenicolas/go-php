FROM golang:1.17.8-bullseye

# The full PHP version to target, i.e. "7.1.10".
ARG PHP_VERSION

# Whether or not to build PHP as a static library.
ARG STATIC=false

ARG PHP_VERSION_INSTALL

# Build variables.
ENV PHP_LDFLAGS="-Wl,-O1 -Wl,--hash-style=both -pie"
ENV PHP_CFLAGS="-fstack-protector-strong -fpic -fpie -O2"
ENV PHP_CPPFLAGS="${PHP_CFLAGS}"
ENV PHP_VI="${PHP_VERSION_INSTALL}"
# Fetch PHP source code. This step does not currently validate keys or checksums, as this process
# will eventually transition to using the base `php` Docker images.
ENV FETCH_DEPS="apt-utils software-properties-common apt-transport-https lsb-release ca-certificates curl"
RUN set -xe && \
    { \
    echo 'Package: *php*'; \
    echo 'Pin: release a=stable-security'; \
    echo 'Pin-Priority: -1'; \
    } > /etc/apt/preferences.d/no-debian-php;\
    apt-get update && \
    apt-get install -y --no-install-recommends ${FETCH_DEPS} && \
    apt-get update

ENV BUILD_DEPS="dpkg-dev ${PHP_VI}-dev ${PHP_VI}-common ${PHP_VI}-embed ${PHP_VI}-cli ${PHP_VI}-opcache lib${PHP_VI}-embed ${PHP_VI}-readline ${PHP_VI}-opcache ${PHP_VI}-xml ${PHP_VI} php-common lib${PHP_VI}-embed-dbgsym"

RUN curl -vksSLo /usr/share/keyrings/deb.sury.org-php.gpg https://packages.sury.org/php/apt.gpg &&\
    echo "deb [signed-by=/usr/share/keyrings/deb.sury.org-php.gpg] https://packages.sury.org/php/ $(lsb_release -sc) main" > /etc/apt/sources.list.d/php.list && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y && \
    export CFLAGS="${PHP_CFLAGS}" CPPFLAGS="${PHP_CPPFLAGS}" LDFLAGS="${PHP_LDFLAGS}"; \
    apt-get install -y --no-install-recommends ${BUILD_DEPS} &&\
    ln -sT "/usr/include/php/$(ls /usr/include/php)" /usr/include/php/phpsrc
COPY php-tools/docker-entrypoint.sh /root/
RUN export PHPVERSIONID=$(ls /usr/include/php) && \
    export arch="$(dpkg-architecture --query DEB_BUILD_GNU_TYPE)" &&\
    export multiarch="$(dpkg-architecture --query DEB_BUILD_MULTIARCH)" && \
    echo 'export PHPVERSIONID=$(ls /usr/include/php)' >> /etc/profile && \
    chmod +x /root/docker-entrypoint.sh;
CMD ["/bin/sh", "-c"]

