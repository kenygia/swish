// Copyright 2009, 2010, 2012, 2016 Alexander Lamaison

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SWISH_TEST_OPENSSH_FIXTURE_OPENSSH_FIXTURE_HPP
#define SWISH_TEST_OPENSSH_FIXTURE_OPENSSH_FIXTURE_HPP

#include <ssh/filesystem/path.hpp>

#include <boost/filesystem.hpp> // path

#include <string>

namespace test
{

/**
 * Fixture that starts and stops an OpenSSH server.
 */
class openssh_fixture
{
public:
    openssh_fixture();
    virtual ~openssh_fixture();

    std::string host() const;
    std::string user() const;
    std::wstring whost() const;
    std::wstring wuser() const;
    int port() const;
    std::string password() const;
    std::wstring wpassword() const;
    ::ssh::filesystem::path sandbox() const;
    ::ssh::filesystem::path absolute_sandbox() const;
    ::ssh::filesystem::path new_file_in_sandbox() const;
    ::ssh::filesystem::path
    new_file_in_sandbox(const ::ssh::filesystem::path& name) const;
    ::ssh::filesystem::path new_directory_in_sandbox() const;
    boost::filesystem::path private_key_path() const;
    boost::filesystem::path public_key_path() const;
    boost::filesystem::path wrong_private_key_path() const;
    boost::filesystem::path wrong_public_key_path() const;

private:
    std::string m_container_id;
    std::string m_host;
    int m_port;

    std::string ask_docker_for_host() const;
    int ask_docker_for_port() const;
};
}

#endif
