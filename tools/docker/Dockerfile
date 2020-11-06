FROM alpine:latest as builder
WORKDIR /usr/src/app
RUN apk add --no-cache --no-progress build-base git
RUN git clone --depth=1 --branch v5.0.0-alpha15 https://github.com/premake/premake-core.git extern/premake
RUN cd extern/premake && make -f Bootstrap.mak linux
COPY extern lib src premake5.lua /usr/src/app
RUN ./extern/premake/bin/release/premake5 gmake2
RUN make CC=gcc CFLAGS=-O3 config=release

FROM alpine:latest as runner
COPY --from=builder /usr/src/app/bin/release/kobalt /usr/bin
CMD ["kobalt"]
