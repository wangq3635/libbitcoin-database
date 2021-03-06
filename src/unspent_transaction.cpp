/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/database/unspent_transaction.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace database {

using namespace bc::chain;

unspent_transaction::unspent_transaction(unspent_transaction&& other)
  : height_(other.height_),
    is_coinbase_(other.is_coinbase_),
    hash_(std::move(other.hash_)),
    outputs_(other.outputs_)
{
}

unspent_transaction::unspent_transaction(const unspent_transaction& other)
  : height_(other.height_),
    is_coinbase_(other.is_coinbase_),
    hash_(other.hash_),
    outputs_(other.outputs_)
{
}

unspent_transaction::unspent_transaction(const hash_digest& hash)
  : height_(0),
    is_coinbase_(false),
    hash_(hash),
    outputs_(std::make_shared<output_map>())
{
}

unspent_transaction::unspent_transaction(const output_point& point)
  : unspent_transaction(point.hash())
{
}

unspent_transaction::unspent_transaction(const chain::transaction& tx,
    size_t height)
  : height_(height),
    is_coinbase_(tx.is_coinbase()),
    hash_(tx.hash()),
    outputs_(std::make_shared<output_map>())
{
    const auto& outputs = tx.outputs();
    const auto size = safe_unsigned<uint32_t>(outputs.size());
    outputs_->reserve(size);

    // TODO: consider eliminating the byte buffer in favor of ops::list only.
    for (uint32_t index = 0; index < size; ++index)
        (*outputs_)[index] = outputs[index];
}

const hash_digest& unspent_transaction::hash() const
{
    return hash_;
}

size_t unspent_transaction::height() const
{
    return height_;
}

bool unspent_transaction::is_coinbase() const
{
    return is_coinbase_;
}

unspent_transaction::output_map_ptr unspent_transaction::outputs() const
{
    return outputs_;
}

// For the purpose of bimap identity only the tx hash matters.
bool unspent_transaction::operator==(const unspent_transaction& other) const
{
    return hash_ == other.hash_;
}

unspent_transaction& unspent_transaction::operator=(
    unspent_transaction&& other)
{
    height_ = other.height_;
    is_coinbase_ = other.is_coinbase_;
    hash_ = std::move(other.hash_);
    outputs_ = other.outputs_;
    return *this;
}

unspent_transaction& unspent_transaction::operator=(
    const unspent_transaction& other)
{
    height_ = other.height_;
    is_coinbase_ = other.is_coinbase_;
    hash_ = other.hash_;
    outputs_ = other.outputs_;
    return *this;
}

} // namespace database
} // namespace libbitcoin
