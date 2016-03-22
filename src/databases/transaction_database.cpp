/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/database/databases/transaction_database.hpp>

#include <cstddef>
#include <cstdint>
#include <boost/filesystem.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/result/transaction_result.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 100000000;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

transaction_database::transaction_database(const path& map_filename)
  : lookup_file_(map_filename), 
    lookup_header_(lookup_file_, number_buckets),
    lookup_manager_(lookup_file_, header_size),
    lookup_map_(lookup_header_, lookup_manager_)
{
    BITCOIN_ASSERT(REMAP_ADDRESS(lookup_file_.access()) != nullptr);
}

void transaction_database::create()
{
    lookup_file_.resize(initial_map_file_size);
    lookup_header_.create();
    lookup_manager_.create();
}

void transaction_database::start()
{
    lookup_header_.start();
    lookup_manager_.start();
}

bool transaction_database::stop()
{
    return lookup_file_.stop();
}

transaction_result transaction_database::get(const hash_digest& hash) const
{
    const auto memory = lookup_map_.find(hash);
    return transaction_result(memory);
}

void transaction_database::store(size_t height, size_t index,
    const chain::transaction& tx)
{
    // Write block data.
    const auto key = tx.hash();
    const auto tx_size = tx.serialized_size();

    BITCOIN_ASSERT(height <= max_uint32);
    const auto hight32 = static_cast<size_t>(height);

    BITCOIN_ASSERT(index <= max_uint32);
    const auto index32 = static_cast<size_t>(index);

    BITCOIN_ASSERT(tx_size <= max_size_t - 4 - 4);
    const auto value_size = 4 + 4 + static_cast<size_t>(tx_size);

    auto write = [&hight32, &index32, &tx](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_4_bytes_little_endian(hight32);
        serial.write_4_bytes_little_endian(index32);
        serial.write_data(tx.to_data());
    };
    lookup_map_.store(key, write, value_size);
}

void transaction_database::remove(const hash_digest& hash)
{
    DEBUG_ONLY(bool success =) lookup_map_.unlink(hash);
    BITCOIN_ASSERT(success);
}

void transaction_database::sync()
{
    lookup_manager_.sync();
}

} // namespace database
} // namespace libbitcoin