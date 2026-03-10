#pragma once

/*
 * DEV DONATION SETTING
 * Percentage of hashing power donated to the original developers (fireice-uk, psychocrypt).
 * Default is 0% because the original dev donation pool (donate.xmr-stak.net) is no longer
 * maintained; connecting to it would waste hashrate and can cause connection errors.
 * Set to e.g. 2.0 / 100.0 only if a maintained donation pool is configured in executor.cpp.
 *
 * To support the original authors, consider a one-time donation to their wallets:
 * fireice-uk:
 * 4581HhZkQHgZrZjKeCfCJxZff9E3xCgHGF25zABZz7oR71TnbbgiS7sK9jveE6Dx6uMs2LwszDuvQJgRZQotdpHt1fTdDhk
 * psychocrypt:
 * 43NoJVEXo21hGZ6tDG6Z3g4qimiGdJPE6GRxAmiWwm26gwr62Lqo7zRiCJFSBmbkwTGNuuES9ES5TgaVHceuYc4Y75txCTU
 */

constexpr double fDevDonationLevel = 0.0;
