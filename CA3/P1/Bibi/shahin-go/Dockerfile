FROM golang:alpine AS builder
WORKDIR /app
COPY shahin.go .
ENV GO111MODULE=off
RUN go build -o main .

FROM alpine 
WORKDIR /app
COPY --from=builder /app/main .
EXPOSE 80
ENTRYPOINT ["./main"]

