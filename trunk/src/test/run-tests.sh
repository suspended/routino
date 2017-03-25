#!/bin/sh

# Run with a debugger or not?

debugger=valgrind
debugger=

# Overall status

status=true

# Functions for running tests

run_a_test ()
{
    script=$1
    shift

    if ./$script $@ ; then
        echo "... passed"
    else
        echo "... FAILED"
        status=false
    fi
}

compare_results ()
{
    if diff -q -r $1 $2; then
        echo "... matched"
    else
        echo "... match FAILED"
        status=false
    fi
}


# Initial informational message

echo ""
./is-fast-math message


# Loop round the different test types

TEST_DEBUGGER=$debugger
export TEST_DEBUGGER

for type in 1 2 3; do

    case $type in
        1)
            suffix=""
            arg=""
            description=""
            ;;
        2)
            suffix="+lib"
            arg="lib"
            description="libroutino"
            ;;
        3)
            suffix="-pruned"
            arg="prune"
            description="pruned"
            ;;
    esac

    # Normal mode

    for script in $@; do
        echo ""
        echo "Testing: $script (non-slim, $description) ... "
        run_a_test $script fat $arg
    done

    # Normal mode

    for script in $@; do
        echo ""
        echo "Testing: $script (slim, $description) ... "
        run_a_test $script slim $arg
    done

    # Check results

    if $status; then
        echo "Success: all tests passed"
    else
        echo "Warning: Some tests FAILED"
        exit 1
    fi

    # Compare normal/slim results

    echo ""

    echo "Comparing: slim and non-slim results ($description) ... "

    compare_results fat$suffix slim$suffix

    # Check comparison

    if $status; then
        echo "Success: slim and non-slim results match"
    else
        echo "Warning: slim and non-slim results are different"
        exit 1
    fi

done

exit 0
