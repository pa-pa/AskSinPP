DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BASEDIR="$(realpath $DIR/../../..)"
cd $DIR

AES_FLAGS="-DUSE_AES -DHM_DEF_KEY=0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10 -DHM_DEF_KEY_INDEX=0"

####################################
# exec test-runners
# params:
#   - Board FQDN
#   - boolean: compile with AES
#   - ...Sketches
# globals:
#   - AVG_BYTES
#   - AVG_BYTES_AES
####################################
function runTests {
  [ -z "AVG_BYTES" ] && AVG_BYTES=0
  [ -z "AVG_BYTES_AES" ] && AVG_BYTES_AES=0
  local BOARD=$1
  shift
  local AES=${1:-false}
  shift
  local SKETCHES=("$@")
  local HAS_ERROR=0
  $AES && local FLAGS="${AES_FLAGS}"
  local FILE
  local OUT
  local BYTES=0
  for FILE in "${SKETCHES[@]}"; do
    echo "Compiling $(basename $FILE)"
    OUT=$(arduino-cli compile \
      --clean \
      --quiet \
      -b "${BOARD}" \
      --build-property "compiler.cpp.extra_flags=${FLAGS}" \
      $FILE)
    if [ $? -ne 0 ]; then
      HAS_ERROR=1
    else
      BYTES=$(( $BYTES + $(echo $OUT | grep -oP '(?<=Sketch uses )([0-9]+)') ))
    fi
    echo "$OUT"
    echo
  done
  if $AES ; then
    AVG_BYTES_AES=$(( $BYTES / ${#SKETCHES[@]} ))
  else
    AVG_BYTES=$(( $BYTES / ${#SKETCHES[@]} ))
  fi
  return $HAS_ERROR
}


####################################
# find sketches with comment line
# // ci-test=yes board=...
# params:
#   - board name
# globals:
#   - SKETCHES
#   - SKETCHES_AES
#   - SKETCH_PATHES
####################################
function findSketches {
  SKETCHES=()
  SKETCHES_AES=()
  local BOARD=$1
  local DEFAULT_IFS=$IFS
  local FILE
  local LINE
  IFS=$(echo -en "\n\b")
  for FILE in $(find "${SKETCH_PATHES[@]}" -type f -name *.ino); do
    LINE=$(grep -E "^// ci-test=yes.*board=${BOARD}" "${FILE}")
    [ $? -gt 0 ] && continue
    AES=$(echo $LINE | grep -qF 'aes=yes' && echo true || echo false)
    $AES && SKETCHES_AES+=($FILE) || SKETCHES+=($FILE)
  done
  IFS=$DEFAULT_IFS
}


####################################
# Clone/Pull GIT-Repos in parallel
# params:
#   ... Repo URLs
####################################
function downloadGitRepos {
  local GIT_REPOS=("$@")
  local REPO_URL
  local REPO
  for REPO_URL in ${GIT_REPOS[*]}; do
    REPO="$(basename $REPO_URL | cut -d. -f1)"
    if [ -e "$REPO" ] ; then
      echo "Pull changes from $REPO"
      (cd $REPO && git pull -q --depth 1) &
    else
      echo "Clone from $REPO"
      git clone -q --no-tags --depth 1 "$REPO_URL" &
    fi
  done
  wait
}
