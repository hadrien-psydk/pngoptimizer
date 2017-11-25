# Use Ubuntu as build environment
FROM ubuntu:17.10 AS build

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      libgtk-3-dev

# Copy the source code & build the application
COPY . /app
RUN cd /app/projects/pngoptimizercl && make CONFIG=release

# Use fresh Ubuntu as run environment
FROM ubuntu:17.10

# Retrieve the compiled binary from previous image
COPY --from=build /app/projects/pngoptimizercl/linux-release/pngoptimizercl /usr/local/bin/

# Use as an entrypoint for easier usage
ENTRYPOINT ["/usr/local/bin/pngoptimizercl"]
