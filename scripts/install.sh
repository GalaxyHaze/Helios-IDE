#!/usr/bin/env bash
set -euo pipefail

REPO="GalaxyHaze/Helios-IDE"
VERSION=""
INSTALL_ROOT="${HOME}/.local/opt/helios"
BIN_DIR="${HOME}/.local/bin"

usage() {
  echo "Usage: $0 [version]"
  echo "Installs Helios from GitHub releases into ${INSTALL_ROOT}/<version>"
  exit 1
}

for arg in "$@"; do
  case "$arg" in
    -h|--help) usage ;;
    *) VERSION="$arg" ;;
  esac
done

detect_latest_version() {
  local api_url="https://api.github.com/repos/${REPO}/releases/latest"
  if [ -n "${GITHUB_TOKEN:-}" ]; then
    VERSION="$(curl -fsSL -H "Authorization: token ${GITHUB_TOKEN}" "${api_url}" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')"
  else
    VERSION="$(curl -fsSL "${api_url}" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')"
  fi

  if [ -z "${VERSION}" ]; then
    echo "Error: could not detect the latest Helios release." >&2
    exit 1
  fi
}

if [ -z "${VERSION}" ]; then
  echo "No version specified. Fetching latest release..."
  detect_latest_version
fi

os="$(uname -s)"
arch="$(uname -m)"
asset_name=""

case "${os}" in
  Linux*)
    case "${arch}" in
      x86_64) asset_name="helios-linux-amd64.tar.gz" ;;
      aarch64|arm64) asset_name="helios-linux-arm64.tar.gz" ;;
      *) echo "Unsupported Linux architecture: ${arch}" >&2; exit 1 ;;
    esac
    ;;
  Darwin*)
    asset_name="helios-macos.zip"
    ;;
  *)
    echo "Unsupported OS: ${os}" >&2
    exit 1
    ;;
esac

download_url="https://github.com/${REPO}/releases/download/${VERSION}/${asset_name}"
install_dir="${INSTALL_ROOT}/${VERSION}"
tmp_dir="$(mktemp -d)"
archive_path="${tmp_dir}/${asset_name}"

cleanup() {
  rm -rf "${tmp_dir}"
}
trap cleanup EXIT

mkdir -p "${install_dir}" "${BIN_DIR}"

echo "Downloading ${download_url}..."
curl -fsSL "${download_url}" -o "${archive_path}"

rm -rf "${install_dir:?}/"*

case "${asset_name}" in
  *.tar.gz)
    tar -xzf "${archive_path}" -C "${install_dir}"
    ;;
  *.zip)
    unzip -oq "${archive_path}" -d "${install_dir}"
    ;;
esac

if [ ! -f "${install_dir}/Helios" ]; then
  echo "Warning: Helios binary was not found at ${install_dir}/Helios after extraction." >&2
else
  chmod +x "${install_dir}/Helios"
  ln -sfn "${install_dir}/Helios" "${BIN_DIR}/helios"
fi

cat <<EOF
Helios ${VERSION} installed to ${install_dir}

Command shim:
  ${BIN_DIR}/helios

Note:
  The current Unix release artifacts are not yet fully self-contained with Qt runtime
  deployment, so your system still needs a compatible Qt runtime available.
EOF
