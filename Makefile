# Package options.
NAME        := gophp
DESCRIPTION := PHP bindings for the Go programming language
IMPORT_PATH := github.com/sailenicolas/$(NAME)
VERSION     := $(shell git describe --tags --always --dirty="-dev")

# Generic build options.
PHP_VERSION		:= $(firstword $(shell php -i | grep "PHP Version" | sed -e 's/PHP Version =>//g'))
STATIC			:= false
DOCKER_IMAGE	:= sailenicolas/$(NAME):
BUILD_IMAGE_LSB	:= $(shell lsb_release -si)
# Go build options.
GO   := go
TAGS := -tags $(BUILD_IMAGE_LSB),$(if $(findstring true, $(STATIC)), static,)php$(word 1,$(subst ., ,$(PHP_VERSION)))

# Install options.
PREFIX := /usr
# Default Makefile options.
VERBOSE := true

# Variables to pass down to sub-invocations of 'make'.
MAKE_OPTIONS := PHP_VERSION=$(PHP_VERSION) GO=$(GO) PREFIX=$(PREFIX) VERBOSE=$(VERBOSE) STATIC=$(STATIC)

## Build binary distribution for library.
build: .build
	@echo "Building '$(NAME)'..."
	$Q $(GO) install $(if $(VERBOSE),-v) $(TAGS) $(IMPORT_PATH)

## Run test for all local packages or specified PACKAGE.
test: .build
	@echo "Running tests for '$(NAME)'..."
	@echo "Running tests for '$(VERSIONID)'..."
	$Q $(GO) test $(if $(VERBOSE),-v) $(TAGS) $(if $(PACKAGE),$(PACKAGE),$(PACKAGES))
	@echo "Running 'vet' for '$(NAME)'..."
	$Q $(GO) vet $(if $(VERBOSE),-v) $(TAGS) $(if $(PACKAGE),$(PACKAGE),$(PACKAGES))

## Create test coverage report for all local packages or specified PACKAGE.
cover: .build
	@echo "Creating code coverage report for '$(NAME)'..."
	$Q rm -Rf .build/tmp && mkdir -p .build/tmp
	$Q for pkg in $(if $(PACKAGE),$(PACKAGE),$(PACKAGES)); do                                    \
	       name=`echo $$pkg.cover | tr '/' '.'`;                                                 \
	       imports=`go list -f '{{ join .Imports " " }}' $$pkg`;                                 \
	       coverpkg=`echo "$$imports $(PACKAGES)" | tr ' ' '\n' | sort | uniq -d | tr '\n' ','`; \
	       $(GO) test $(if $(VERBOSE),-v) $(TAGS) -coverpkg $$coverpkg$$pkg -coverprofile .build/tmp/$$name $$pkg; done
	$Q awk "$$COVERAGE_MERGE" .build/tmp/*.cover > .build/tmp/cover.merged
	$Q $(GO) tool cover -html .build/tmp/cover.merged -o .build/tmp/coverage.html
	@echo "Coverage report written to '.build/tmp/coverage.html'"
	@echo "Total coverage for '$(NAME)':"
	$Q $(GO) tool cover -func .build/tmp/cover.merged

## Remove temporary files and packages required for build.
clean:
	@echo "Cleaning '$(NAME)'..."
	$Q $(GO) clean -cache
	$Q rm -Rf .build

## Show usage information for this Makefile.
help:
	@printf "$(BOLD)$(DESCRIPTION)$(RESET)\n\n"
	@printf "This Makefile contains tasks for processing auxiliary actions, such as\n"
	@printf "building binaries, packages, or running tests against the test suite.\n\n"
	@printf "$(UNDERLINE)Available Tasks$(RESET)\n\n"
	@awk -F                                                                        \
	    ':|##' '/^##/ {c=$$2; getline; printf "$(BLUE)%10s$(RESET) %s\n", $$1, c}' \
	    $(MAKEFILE_LIST)
	@printf "\n"

.PHONY: build test cover clean

.DEFAULT:
	$Q $(MAKE) -s -f $(MAKEFILE) help

# Pull or build Docker image for PHP version specified.
docker-image:
	$Q docker image pull $(DOCKER_IMAGE)8.1.3 ||                \
	   docker build --build-arg=PHP_VERSION="8.1.3" --build-arg=STATIC=$(STATIC)  \
	   --build-arg=PHP_VERSION_INSTALL=php8.1 \
	  -t $(DOCKER_IMAGE)8.1.3 -f "php-tools/Dockerfile" .
	$Q docker image pull $(DOCKER_IMAGE)8.0.17 ||                \
	   docker build --build-arg=PHP_VERSION="8.0.17" --build-arg=STATIC=$(STATIC)  \
	   --build-arg=PHP_VERSION_INSTALL=php8.0 \
	  -t $(DOCKER_IMAGE)8.0.17 -f "php-tools/Dockerfile" .

# Run Make target in Docker container. For instance, to run 'test', call as 'docker-test'.
docker-%: docker-image
	$Q docker run --rm -e GOPATH="/tmp/go"                                  \
	              -v "$(CURDIR):/tmp/go/src/$(IMPORT_PATH)" $(DOCKER_IMAGE)$(PHP_VERSION) \
	                 "$(MAKE) -C /tmp/go/src/$(IMPORT_PATH) $(word 2,$(subst -, ,$@)) $(MAKE_OPTIONS)"

.build:
	$Q mkdir -p "$(dir .build/src/$(IMPORT_PATH))" && touch $@
	$Q ln -s ../../../.. ".build/src/$(IMPORT_PATH)"

MAKEFILE := $(lastword $(MAKEFILE_LIST))
Q := $(if $(VERBOSE),,@)

PACKAGES = $(shell (                                                    \
	cd $(CURDIR)/.build/src/$(IMPORT_PATH) &&                \
	GOPATH=$(CURDIR)/.build go list ./... | grep -v "vendor" \
))

export GOPATH := $(CURDIR)/.build

BOLD      = \033[1m
UNDERLINE = \033[4m
BLUE      = \033[36m
RESET     = \033[0m

define COVERAGE_MERGE
/^mode: (set|count|atomic)/ {
	if ($$2 == "set") mode = "set"
	next
}
/^mode: / {
	printf "Unknown mode '%s' in %s, line %d", $$2, FILENAME, FNR | "cat >&2"
	exit 1
}
{
	val = $$NF; $$NF = ""
	blocks[$$0] += val
}
END {
	printf "mode: %s\n", (mode == "set") ? "set" : "count"
	for (b in blocks) {
		printf "%s%d\n", b, (mode == "set" && blocks[b] > 1) ? 1 : blocks[b]
	}
}
endef
export COVERAGE_MERGE
