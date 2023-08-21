#!/bin/bash

set -Eeuo pipefail

# Allow scripts to "escape" to top-level
TOP_LEVEL="$(git rev-parse --show-toplevel)"

# Directory with test scripts
TEST_DIR="$(dirname "$0")/tests"
EXIT_ZERO=false
DEBUG=false

# Default regex for file names - run-parts equivalent
DEFAULT_REGEX='^[a-zA-Z0-9_-]+$'
REGEX="$DEFAULT_REGEX"


run_tests() {
    # Guard from pre-commit's `str.strip()`
    printf %b '\u200b'

    # Counter for failed tests
    local FAIL_COUNT=0

    local DIR=

    if command -V realpath &> /dev/null ; then
        DIR="$(realpath "${TEST_DIR}" --relative-to "${TOP_LEVEL}")/"
    fi

    # Run tests
    for TEST_SCRIPT in "${DIR}"*; do
        local RESULT=99

        if ! [[ "$(basename "${TEST_SCRIPT}")" =~ ${REGEX} ]]; then
            "${DEBUG}" && {
                >&2 echo "${TEST_SCRIPT} does not match ${REGEX}"
            }
            continue
        fi

        if ! [ -x "${TEST_SCRIPT}" ]; then
            >&2 echo -e "\xE2\x9D\x8C ${TEST_SCRIPT} is not executable!"
            continue
        fi

        {
            TOP_LEVEL="${TOP_LEVEL}" "./${TEST_SCRIPT}" && RESULT=$? || RESULT=$?
        } > >(sed 's/^/   /') 2> >(sed 's/^/   /' >&2)

        # Display results with flashy stuff
        if [ "${RESULT}" -eq 0 ]; then
            echo -e "\xE2\x9C\x85 ${TEST_SCRIPT}"  # Check mark for success
        else
            echo -e "\xE2\x9D\x8C ${TEST_SCRIPT} (\$?:${RESULT})"  # Cross mark for failure
            FAIL_COUNT="$((FAIL_COUNT + 1))"
        fi
    done

    return "${FAIL_COUNT}"
}

# Only run the main function if the script isn't being sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    UNKNOWN_ARGS=()
    # https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
    while [[ $# -gt 0 ]] ; do
        key="$1"

        case ${key} in
            --debug)
                DEBUG=true
            ;;
            --exit-0)
                EXIT_ZERO=true
            ;;
            --regex)
                REGEX="$2"
                shift  # past argument
            ;;
            *)    # unknown option
                UNKNOWN_ARGS+=("$1")
            ;;
        esac
        shift # past argument
    done

    if [ "${#UNKNOWN_ARGS[@]}" -ne 0 ] ; then
        {
            echo "Unknown arguments found:"
            for arg in "${UNKNOWN_ARGS[@]}" ; do
                printf '"%s" ' "${arg}"
            done
            echo
        } >&2
        exit 125
    fi

    (
        if command -V realpath &> /dev/null ; then
            CD_TO="${TOP_LEVEL}"
        else
            CD_TO="${TEST_DIR}"
        fi

        if ! cd "${CD_TO}" ; then
            >&2 echo "Cannot cd to '${CD_TO}' (\$?:$?)!"
            exit 124
        fi

        run_tests "$@"
        FAIL_COUNT=$?

        # Exit with the number of failed tests or 0 if --exit-0 is given
        if "${EXIT_ZERO}"; then
            [ "${FAIL_COUNT}" -ne 0 ] && >&2 echo "Tests failed: ${FAIL_COUNT}, but exiting zero"
            exit 0
        fi

        exit "${FAIL_COUNT}"
    )
fi
