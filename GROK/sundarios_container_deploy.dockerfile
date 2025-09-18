# SundarOS Container Deployment
FROM python:3.9-slim

# Set working directory
WORKDIR /sundarios

# Install system dependencies
RUN apt-get update && apt-get install -y \
    gcc \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy SundarOS files
COPY . .

# Install Python dependencies
RUN pip install --no-cache-dir \
    asyncio \
    sqlite3 \
    psutil \
    threading

# Set environment variables
ENV SUNDARIOS_PLATFORM=container
ENV SUNDARIOS_MODE=hosted
ENV PYTHONPATH=/sundarios

# Expose ports
EXPOSE 8080 8443

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD python3 -c "import sys; sys.exit(0)"

# Run SundarOS
CMD ["python3", "launch_sundarios.py"]
