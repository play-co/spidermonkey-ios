#!/bin/sh

section () {
  echo "------------------------------------------------------------------------"
  echo "Creating $1 version..."
  echo "------------------------------------------------------------------------"
}

fail_with_message () {
  echo "\n\tError: $1"; exit;
}

assert_success() {
  if (($1)); then
    fail_with_message "$2"
  fi
}

