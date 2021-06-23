FROM alpine:3.13 as builder
ADD api /api/
# dependencies
RUN apk add --no-cache git libgfortran gfortran cmake dos2unix patch g++ bash make file openjdk11-jdk openjdk11-jre
# build coin-or libraries
WORKDIR /api/extern/coinor
ENV ADD_FFLAGS=-fallow-argument-mismatch
SHELL ["/bin/bash", "-c"]
ENV ADD_CXXFLAGS=-lrt
RUN ./coinbrew build Cbc@2.10 --tests none --prefix linux64/dist
# build clp-wrapper
RUN mkdir /api/build
WORKDIR /api/build
ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk
RUN cmake ..
RUN cmake --build .
# test wrapper
WORKDIR /api/build/test
RUN ./CLPTest
WORKDIR /api/build/src
# copy all necessary binaries to one place
RUN mkdir binaries
RUN cp libCLPModelWrapper_JNI.so $(ldd libCLPModelWrapper_JNI.so | grep -v 'musl' | sed -E 's/.*=> ([^ ]+).*/\1/') binaries

FROM azul/zulu-openjdk-alpine:13.0.7-jre-headless
RUN mkdir -p app/native
COPY --from=builder /api/build/src/binaries app/native
