#!/bin/bash
# NIO Racing Plus - Automated Asset Download Script
# Downloads free assets from ambientCG, Poly Haven, Kenney, and Freesound

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASSETS_DIR="${PROJECT_DIR}/Assets"
CONTENT_DIR="${PROJECT_DIR}/Content"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${BLUE}[STEP]${NC} $1"; }
log_download() { echo -e "${CYAN}[DOWNLOAD]${NC} $1"; }

# Create directories
create_directories() {
    log_step "Creating asset directories..."

    mkdir -p "${ASSETS_DIR}/Textures/PBR/Asphalt"
    mkdir -p "${ASSETS_DIR}/Textures/PBR/Concrete"
    mkdir -p "${ASSETS_DIR}/Textures/PBR/Metal"
    mkdir -p "${ASSETS_DIR}/Textures/PBR/Ground"
    mkdir -p "${ASSETS_DIR}/Textures/PBR/Rubber"
    mkdir -p "${ASSETS_DIR}/Textures/PBR/Glass"
    mkdir -p "${ASSETS_DIR}/Textures/PBR/Wood"
    mkdir -p "${ASSETS_DIR}/HDR"
    mkdir -p "${ASSETS_DIR}/Audio/SFX"
    mkdir -p "${ASSETS_DIR}/Audio/Motor"
    mkdir -p "${ASSETS_DIR}/Audio/Music"
    mkdir -p "${ASSETS_DIR}/Audio/UI"
    mkdir -p "${CONTENT_DIR}/Textures/PBR"
    mkdir -p "${CONTENT_DIR}/Audio/SFX"
    mkdir -p "${CONTENT_DIR}/Audio/Motor"
    mkdir -p "${CONTENT_DIR}/Audio/Music"
    mkdir -p "${CONTENT_DIR}/Audio/UI"
    mkdir -p "${CONTENT_DIR}/Maps"

    log_info "Directories created"
}

# Download file with retry
download_file() {
    local url="$1"
    local output="$2"
    local max_retries=3
    local retry=0

    while [ $retry -lt $max_retries ]; do
        if curl -L -o "$output" "$url" --connect-timeout 30 --max-time 300 2>/dev/null; then
            return 0
        fi
        retry=$((retry + 1))
        log_warn "Retry $retry/$max_retries for: $url"
        sleep 2
    done

    log_error "Failed to download: $url"
    return 1
}

# Download PBR textures from ambientCG
download_ambientcg_textures() {
    log_step "Downloading PBR textures from ambientCG..."

    # ambientCG provides direct download links for CC0 textures
    # Format: https://ambientcg.com/get?file=NAME_2K-PNG.zip

    local textures=(
        # Asphalt textures
        "Asphalt01|https://ambientcg.com/get?file=Asphalt01_2K-PNG.zip"
        "Asphalt02|https://ambientcg.com/get?file=Asphalt02_2K-PNG.zip"
        "Asphalt03|https://ambientcg.com/get?file=Asphalt03_2K-PNG.zip"
        # Concrete textures
        "Concrete01|https://ambientcg.com/get?file=Concrete01_2K-PNG.zip"
        "Concrete02|https://ambientcg.com/get?file=Concrete02_2K-PNG.zip"
        "Concrete03|https://ambientcg.com/get?file=Concrete03_2K-PNG.zip"
        # Metal textures
        "Metal01|https://ambientcg.com/get?file=Metal01_2K-PNG.zip"
        "Metal02|https://ambientcg.com/get?file=Metal02_2K-PNG.zip"
        "Metal03|https://ambientcg.com/get?file=Metal03_2K-PNG.zip"
        "MetalPlates01|https://ambientcg.com/get?file=MetalPlates01_2K-PNG.zip"
        # Ground textures
        "Ground01|https://ambientcg.com/get?file=Ground01_2K-PNG.zip"
        "Ground02|https://ambientcg.com/get?file=Ground02_2K-PNG.zip"
        "Gravel01|https://ambientcg.com/get?file=Gravel01_2K-PNG.zip"
        "Grass01|https://ambientcg.com/get?file=Grass01_2K-PNG.zip"
        "Sand01|https://ambientcg.com/get?file=Sand01_2K-PNG.zip"
        "Rock01|https://ambientcg.com/get?file=Rock01_2K-PNG.zip"
        # Rubber/Tire textures
        "Rubber01|https://ambientcg.com/get?file=Rubber01_2K-PNG.zip"
        # Wood textures
        "Wood01|https://ambientcg.com/get?file=Wood01_2K-PNG.zip"
        "Wood02|https://ambientcg.com/get?file=Wood02_2K-PNG.zip"
        # Fabric
        "Fabric01|https://ambientcg.com/get?file=Fabric01_2K-PNG.zip"
    )

    for entry in "${textures[@]}"; do
        IFS='|' read -r name url <<< "$entry"
        local output="${ASSETS_DIR}/Textures/PBR/${name}_2K.zip"

        if [ -f "$output" ]; then
            log_info "Already downloaded: $name"
            continue
        fi

        log_download "Downloading $name..."
        if download_file "$url" "$output"; then
            # Extract to appropriate directory
            local category=""
            if [[ "$name" == Asphalt* ]]; then
                category="Asphalt"
            elif [[ "$name" == Concrete* ]]; then
                category="Concrete"
            elif [[ "$name" == Metal* ]]; then
                category="Metal"
            elif [[ "$name" == Ground* ]] || [[ "$name" == Grass* ]] || [[ "$name" == Sand* ]] || [[ "$name" == Rock* ]] || [[ "$name" == Gravel* ]]; then
                category="Ground"
            elif [[ "$name" == Rubber* ]]; then
                category="Rubber"
            elif [[ "$name" == Wood* ]]; then
                category="Wood"
            elif [[ "$name" == Fabric* ]]; then
                category="Ground"
            fi

            mkdir -p "${ASSETS_DIR}/Textures/PBR/${category}/${name}"
            unzip -q -o "$output" -d "${ASSETS_DIR}/Textures/PBR/${category}/${name}" 2>/dev/null || true
            log_info "Extracted: $name -> ${category}/"
        fi
    done

    log_info "PBR textures download complete"
}

# Download HDR skyboxes from Poly Haven
download_polyhaven_hdr() {
    log_step "Downloading HDR skyboxes from Poly Haven..."

    # Poly Haven provides direct download links
    local hdris=(
        "clear sky|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/clear sky_1k.hdr"
        "partly cloudy|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/partly cloudy_1k.hdr"
        "sunset 01|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/sunset 01_1k.hdr"
        "golden hour|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/golden hour_1k.hdr"
        "night 01|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/night 01_1k.hdr"
        "clear night|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/clear night_1k.hdr"
        "overcast|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/overcast_1k.hdr"
        "stormy|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/stormy_1k.hdr"
        "desert|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/desert_1k.hdr"
        "mountain|https://dl.polyhaven.org/file-ph-assets/HDRIs/hdr/1k/mountain_1k.hdr"
    )

    for entry in "${hdris[@]}"; do
        IFS='|' read -r name url <<< "$entry"
        local output="${ASSETS_DIR}/HDR/${name}.hdr"

        if [ -f "$output" ]; then
            log_info "Already downloaded: $name"
            continue
        fi

        log_download "Downloading $name HDR..."
        download_file "$url" "$output" || log_warn "Failed to download: $name"
    done

    log_info "HDR skyboxes download complete"
}

# Download audio from Kenney
download_kenney_audio() {
    log_step "Downloading audio assets from Kenney..."

    # Kenney provides free CC0 audio packs
    local packs=(
        "ui audio|https://kenney.nl/media/pages/assets/ui-audio/c3e1fef17c-1677578891/Kenney%20UI%20Audio.zip"
        "impact sounds|https://kenney.nl/media/pages/assets/impact-sounds/b0e0e0e0e0-1677578891/Kenney%20Impact%20Sounds.zip"
    )

    for entry in "${packs[@]}"; do
        IFS='|' read -r name url <<< "$entry"
        local output="${ASSETS_DIR}/Audio/${name}.zip"

        if [ -f "$output" ]; then
            log_info "Already downloaded: $name"
            continue
        fi

        log_download "Downloading $name..."
        if download_file "$url" "$output"; then
            unzip -q -o "$output" -d "${ASSETS_DIR}/Audio/" 2>/dev/null || true
            log_info "Extracted: $name"
        fi
    done

    log_info "Kenney audio download complete"
}

# Create placeholder motor sounds (synthesized)
create_placeholder_motor_sounds() {
    log_step "Creating placeholder motor sounds..."

    # Create a Python script to generate synthetic motor sounds
    cat > "${ASSETS_DIR}/Audio/generate_motor_sounds.py" << 'PYEOF'
#!/usr/bin/env python3
"""
Generate synthetic electric motor sounds for NIO Racing Plus
Uses numpy to create layered sine waves with harmonics
"""

import numpy as np
import struct
import wave
import os

def generate_motor_sound(rpm_range, duration, output_path, character="neutral"):
    """Generate a motor sound loop"""
    sample_rate = 44100
    num_samples = int(sample_rate * duration)

    # Time array
    t = np.linspace(0, duration, num_samples, endpoint=False)

    # Base frequency varies with RPM (electric motor whine)
    # Typical EV motor: 100-800 Hz fundamental
    base_freq_start = rpm_range[0] / 60.0  # Convert RPM to Hz
    base_freq_end = rpm_range[1] / 60.0

    # Frequency sweep
    freq = np.linspace(base_freq_start, base_freq_end, num_samples)

    # Generate harmonics
    signal = np.zeros(num_samples)

    # Fundamental
    phase = 2 * np.pi * np.cumsum(freq) / sample_rate
    signal += 0.4 * np.sin(phase)

    # 2nd harmonic (inverter switching)
    signal += 0.2 * np.sin(2 * phase)

    # 3rd harmonic
    signal += 0.1 * np.sin(3 * phase)

    # 4th harmonic (gear mesh)
    signal += 0.05 * np.sin(4 * phase)

    # Add some noise for realism
    noise = np.random.normal(0, 0.02, num_samples)
    signal += noise

    # Apply character-specific modifications
    if character == "aggressive":
        signal *= 1.2
        signal += 0.15 * np.sin(6 * phase)  # More harmonics
    elif character == "luxury":
        signal *= 0.7
        signal += 0.05 * np.sin(5 * phase)
    elif character == "suv":
        signal *= 0.9
        signal += 0.08 * np.sin(3.5 * phase)

    # Normalize
    signal = signal / np.max(np.abs(signal)) * 0.8

    # Convert to 16-bit PCM
    signal_int = np.int16(signal * 32767)

    # Write WAV file
    with wave.open(output_path, 'w') as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(signal_int.tobytes())

    print(f"Generated: {output_path}")

def main():
    output_dir = os.path.dirname(os.path.abspath(__file__))

    # EP9 - Aggressive hypercar
    generate_motor_sound(
        rpm_range=(800, 15000),
        duration=4.0,
        output_path=os.path.join(output_dir, "Motor/EP9_motor_loop.wav"),
        character="aggressive"
    )

    # ET7 - Luxury sedan
    generate_motor_sound(
        rpm_range=(600, 12000),
        duration=4.0,
        output_path=os.path.join(output_dir, "Motor/ET7_motor_loop.wav"),
        character="luxury"
    )

    # ES7 - SUV
    generate_motor_sound(
        rpm_range=(600, 12000),
        duration=4.0,
        output_path=os.path.join(output_dir, "Motor/ES7_motor_loop.wav"),
        character="suv"
    )

    # Generate tire screech
    sample_rate = 44100
    duration = 2.0
    num_samples = int(sample_rate * duration)
    t = np.linspace(0, duration, num_samples, endpoint=False)

    # Tire screech is a complex mix of frequencies
    screech = np.zeros(num_samples)
    for freq in [2000, 3000, 4000, 5000, 6000]:
        screech += 0.2 * np.sin(2 * np.pi * freq * t + np.random.uniform(0, 2*np.pi))

    screech *= np.exp(-t * 2)  # Decay
    screech = screech / np.max(np.abs(screech)) * 0.7
    screech_int = np.int16(screech * 32767)

    with wave.open(os.path.join(output_dir, "SFX/tire_screech.wav"), 'w') as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(screech_int.tobytes())

    print("Generated: tire_screech.wav")

    # Generate collision sounds
    for i, impact_type in enumerate(["light", "medium", "heavy"]):
        duration = 0.5 + i * 0.2
        num_samples = int(sample_rate * duration)
        t = np.linspace(0, duration, num_samples, endpoint=False)

        # Impact is a burst of noise with resonance
        impact = np.random.normal(0, 1, num_samples) * np.exp(-t * (10 - i*3))
        impact += 0.5 * np.sin(2 * np.pi * 100 * t) * np.exp(-t * 5)
        impact += 0.3 * np.sin(2 * np.pi * 200 * t) * np.exp(-t * 8)

        impact = impact / np.max(np.abs(impact)) * 0.8
        impact_int = np.int16(impact * 32767)

        with wave.open(os.path.join(output_dir, f"SFX/collision_{impact_type}.wav"), 'w') as wav_file:
            wav_file.setnchannels(1)
            wav_file.setsampwidth(2)
            wav_file.setframerate(sample_rate)
            wav_file.writeframes(impact_int.tobytes())

        print(f"Generated: collision_{impact_type}.wav")

    # Generate UI sounds
    for sound_name, freq, duration in [
        ("ui_click", 800, 0.1),
        ("ui_hover", 600, 0.05),
        ("ui_success", 1000, 0.3),
        ("ui_error", 400, 0.2),
        ("countdown_tick", 500, 0.15),
        ("countdown_go", 800, 0.5),
    ]:
        num_samples = int(sample_rate * duration)
        t = np.linspace(0, duration, num_samples, endpoint=False)

        tone = 0.5 * np.sin(2 * np.pi * freq * t)
        tone *= np.exp(-t * 10)

        tone = tone / np.max(np.abs(tone)) * 0.6
        tone_int = np.int16(tone * 32767)

        with wave.open(os.path.join(output_dir, f"UI/{sound_name}.wav"), 'w') as wav_file:
            wav_file.setnchannels(1)
            wav_file.setsampwidth(2)
            wav_file.setframerate(sample_rate)
            wav_file.writeframes(tone_int.tobytes())

        print(f"Generated: {sound_name}.wav")

if __name__ == "__main__":
    main()
PYEOF

    # Run the generator
    cd "${ASSETS_DIR}/Audio"
    python3 generate_motor_sounds.py
    cd "${PROJECT_DIR}"

    # Copy generated sounds to Content
    cp -r "${ASSETS_DIR}/Audio/Motor/"*.wav "${CONTENT_DIR}/Audio/Motor/" 2>/dev/null || true
    cp -r "${ASSETS_DIR}/Audio/SFX/"*.wav "${CONTENT_DIR}/Audio/SFX/" 2>/dev/null || true
    cp -r "${ASSETS_DIR}/Audio/UI/"*.wav "${CONTENT_DIR}/Audio/UI/" 2>/dev/null || true

    log_info "Placeholder motor sounds created"
}

# Create placeholder textures (solid colors for testing)
create_placeholder_textures() {
    log_step "Creating placeholder textures..."

    cat > "${ASSETS_DIR}/Textures/generate_textures.py" << 'PYEOF'
#!/usr/bin/env python3
"""
Generate placeholder textures for NIO Racing Plus
Creates simple colored textures for testing
"""

import struct
import zlib
import os

def create_png(width, height, color, output_path):
    """Create a simple PNG file with solid color"""

    def create_chunk(chunk_type, data):
        chunk = chunk_type + data
        crc = struct.pack('>I', zlib.crc32(chunk) & 0xffffffff)
        return struct.pack('>I', len(data)) + chunk + crc

    # PNG signature
    signature = b'\x89PNG\r\n\x1a\n'

    # IHDR chunk
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    ihdr = create_chunk(b'IHDR', ihdr_data)

    # IDAT chunk
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00'  # Filter type: None
        for x in range(width):
            raw_data += bytes(color)

    compressed = zlib.compress(raw_data)
    idat = create_chunk(b'IDAT', compressed)

    # IEND chunk
    iend = create_chunk(b'IEND', b'')

    # Write file
    with open(output_path, 'wb') as f:
        f.write(signature + ihdr + idat + iend)

    print(f"Created: {output_path}")

def main():
    output_dir = os.path.dirname(os.path.abspath(__file__))

    # Vehicle paint colors
    colors = {
        "NIO_Blue": (0, 100, 200),
        "NIO_Gray": (100, 100, 110),
        "NIO_White": (230, 230, 235),
        "NIO_Black": (20, 20, 25),
        "NIO_Red": (180, 30, 30),
        "NIO_Silver": (180, 180, 190),
        "NIO_Gold": (200, 170, 50),
        "NIO_Green": (30, 150, 80),
    }

    # Create vehicle paint textures
    for name, color in colors.items():
        create_png(256, 256, color, os.path.join(output_dir, f"Vehicle/{name}_D.png"))

    # Create road textures
    road_colors = {
        "Asphalt_Dark": (40, 40, 45),
        "Asphalt_Light": (80, 80, 85),
        "Concrete": (180, 180, 175),
        "Road_Markings": (240, 240, 240),
    }

    for name, color in road_colors.items():
        create_png(512, 512, color, os.path.join(output_dir, f"Road/{name}_D.png"))

    # Create environment textures
    env_colors = {
        "Grass": (50, 120, 40),
        "Dirt": (120, 80, 40),
        "Sand": (200, 180, 120),
        "Rock": (100, 95, 85),
        "Water": (30, 80, 150),
    }

    for name, color in env_colors.items():
        create_png(256, 256, color, os.path.join(output_dir, f"Environment/{name}_D.png"))

    # Create normal map placeholder (flat normal = 128, 128, 255)
    create_png(256, 256, (128, 128, 255), os.path.join(output_dir, "flat_normal.png"))

    # Create roughness map (mid-gray = 0.5 roughness)
    create_png(256, 256, (128, 128, 128), os.path.join(output_dir, "mid_roughness.png"))

    print("All placeholder textures generated!")

if __name__ == "__main__":
    os.makedirs(os.path.dirname(os.path.abspath(__file__)) + "/Vehicle", exist_ok=True)
    os.makedirs(os.path.dirname(os.path.abspath(__file__)) + "/Road", exist_ok=True)
    os.makedirs(os.path.dirname(os.path.abspath(__file__)) + "/Environment", exist_ok=True)
    main()
PYEOF

    # Run the generator
    cd "${ASSETS_DIR}/Textures"
    python3 generate_textures.py
    cd "${PROJECT_DIR}"

    log_info "Placeholder textures created"
}

# Main function
main() {
    echo "=========================================="
    echo "  NIO Racing Plus - Asset Download"
    echo "=========================================="
    echo ""

    create_directories

    echo ""
    log_step "Starting asset downloads..."
    echo ""

    # Download free assets
    download_ambientcg_textures
    download_polyhaven_hdr

    echo ""
    log_step "Generating placeholder assets..."
    echo ""

    # Generate placeholder assets
    create_placeholder_motor_sounds
    create_placeholder_textures

    echo ""
    echo "=========================================="
    echo "  Download Complete!"
    echo "=========================================="
    echo ""
    log_info "Assets downloaded to: ${ASSETS_DIR}"
    log_info "Next steps:"
    log_info "  1. Download NIO vehicle models from Sketchfab (manual)"
    log_info "  2. Open UE5 Editor and run import scripts"
    log_info "  3. Run: python3 Scripts/Editor/run_all.py"
}

main "$@"
