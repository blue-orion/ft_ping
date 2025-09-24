#!/usr/bin/env bash
set -euo pipefail

# ==== Config ====
USERNAME="${1:-takwak}"         # first argument (default: takwak)
EXTRA_PKGS="${EXTRA_PKGS:-git vim make gcc}" # package to install

# ==== Re-exec as root ====
if [ "$(id -u)" -ne 0 ]; then
	if command -v sudo > /dev/null 2>&1; then
		exec sudo -E bash "$0" "$@"
	else
		echo "[i] sudo not installed, try restart using 'su -c'"
		exec su - -c "bash '$0' '$@'"
	fi
fi

# ==== Detect distro / pkg manager ====
OS_ID=""; OS_LIKE=""
if [ -r /etc/os-release ]; then
	# shellcheck disable=SC1091
	. /etc/os-release
	OS_ID="${ID:-}"
	OS_LIKE="${ID_LIKE:-}"
fi

has() { command -v "$1" > /dev/null 2>&1; }

PKG_MGR=""
REFRESH=""
INSTALL=""
BASE_PKGS=""

case "${OS_ID}" in
	debian|ubuntu|raspbian)
		PKG_MGR="apt-get"
		export  DEBIAN_FRONTEND=noninteractive
		REFRESH="apt-get update -y"
		INSTALL="apt-get install -y"
		BASE_PKGS="sudo passwd"
		;;
	alpine)
		PKG_MGR="apk"
		REFRESH="apk update"
		INSTALL="apk add --no-cache"
		BASE_PKGS="sudo shadow"
		;;
	*)
		echo "Failed to load OS type"
		exit 1
		;;
esac

echo "[i] Detected: id=${OS_ID:-unknown} like=${OS_LIKE:-} pkgmgr=$PKG_MGR"

case "$PKG_MGR" in
	apt-get)
		$REFRESH
		$INSTALL ca-certificates > /dev/null 2>&1 || true
		$INSTALL $BASE_PKGS $EXTRA_PKGS
		;;
	apk)
		$REFRESH
		$INSTALL ca-certificates > /dev/null 2>&1 || true
		$INSTALL $BASE_PKGS $EXTRA_PKGS
		update-ca-certificates || true
		;;
esac

# ==== Ensuer user exists ====

if ! id -u "$USERNAME" > /dev/null 2>&1; then
	if has useradd; then
		useradd -m -s /bin/bash "$USERNAME" || useradd -m "$USERNAME"
	elif has adduser; then
		adduser -D "$USERNAME" 2>/dev/null || adduser --disabled-password --gecos "" "$USERNAME"
	else
		echo "useradd/adduser not installed" >&2
		exit 1
	fi
fi

# ==== Add to sudo group ====
SUDO_GROUP="sudo"

if has usermod; then
	usermod -aG "$SUDO_GROUP" "$USERNAME" || true
elif has adduser; then
	adduser "$USERNAME" "$SUDO_GROUP" || true
fi

# ==== Sudoers drop-in ====
SUDOERS_DIR="/etc/sudoers.d"
mkdir -p "$SUDOERS_DIR"
SUDOERS_FILE="$SUDOERS_DIR/$USERNAME"

printf "%s ALL=(ALL:ALL) ALL\n" "$USERNAME" > "$SUDOERS_FILE"
chmod 0440 "$SUDOERS_FILE"

if has visudo; then
	visudo -cf "$SUDOERS_FILE" > /dev/null
fi

# ==== Clone ft_ping repository ====
echo "git clone https://github.com/blue-orion/ft_ping"

if has git; then
	git clone https://github.com/blue-orion/ft_ping
fi

echo "Setup done for user: $USERNAME"
