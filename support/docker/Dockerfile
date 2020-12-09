FROM alpine:latest as builder
WORKDIR /usr/src/app
RUN apk add --no-cache --no-progress build-base git clang
RUN git clone --depth=1 --branch v5.0.0-alpha15 https://github.com/premake/premake-core.git extern/premake
RUN cd extern/premake && make -f Bootstrap.mak linux
COPY include include
COPY extern extern
COPY lib lib
COPY src src
COPY premake5.lua premake5.lua
RUN ./extern/premake/bin/release/premake5 --cc=clang gmake2
RUN make config=release

FROM alpine:latest as runner
COPY --from=builder /usr/src/app/bin/release/kobalt /usr/bin
CMD ["kobalt"]
