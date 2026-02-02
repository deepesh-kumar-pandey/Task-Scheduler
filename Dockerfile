# Use Alpine Linux as the base image
FROM alpine:latest

# Install build dependencies (g++, make, etc.)
RUN apk add --no-cache build-base

# Set the working directory inside the container
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . .

# Compile the C++ application
# -o task_scheduler: Output file name
# src/*.cpp: Source files
# -I include: Include directory for header files
RUN g++ -o task_scheduler src/*.cpp -I include

# Run the application
CMD ["./task_scheduler"]
