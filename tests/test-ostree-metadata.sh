#!/bin/bash
#
# Copyright (C) 2023 Endless OS Foundation LLC
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

set -euo pipefail

USE_COLLECTIONS_IN_SERVER=yes
USE_COLLECTIONS_IN_CLIENT=yes

. $(dirname $0)/libtest.sh

echo "1..2"

setup_repo

# Ensure the ostree-metadata commit has been created and that the
# summary listing contains it.
assert_has_file repos/test/summary
assert_has_file repos/test/refs/heads/ostree-metadata
ostree --repo=repos/test summary --view > summary-view
assert_file_has_content summary-view "(${COLLECTION_ID}, ostree-metadata)"
${FLATPAK} repo --branches repos/test > branches
assert_file_has_content branches "^ostree-metadata\s"

ok "server repo"

# Ensure a remote with a collection ID defined fetches the
# ostree-metadata commit.
${FLATPAK} ${U} remote-ls test-repo >/dev/null
assert_has_file ${FL_DIR}/repo/refs/remotes/test-repo/ostree-metadata

ok "client repo"
