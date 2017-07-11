/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#include "connect.h"
#include "execute_helper.h"

#include <multipass/cli/argparser.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Connect::run(mp::ArgParser* parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::ExecReply& reply) {
        if (reply.exec_line().empty())
        {
            cout << "received connect reply\n";
            return ReturnCode::Ok;
        }
        else
        {
            std::vector<std::string> cmd_line;
            for (const auto& arg : reply.exec_line())
                cmd_line.push_back(arg);

            return execute_process(cmd_line);
        }
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "connect failed: " << status.error_message() << "\n";
        return ReturnCode::CommandFail;
    };

    return dispatch(&RpcMethod::exec, request, on_success, on_failure);
}

std::string cmd::Connect::name() const { return "connect"; }

QString cmd::Connect::short_help() const
{
    return QStringLiteral("Connect to a running instance");
}

QString cmd::Connect::description() const
{
    return QStringLiteral("Open a prompt on the instance.");
}

mp::ParseCode cmd::Connect::parse_args(mp::ArgParser* parser)
{
    parser->addPositionalArgument("name", "Name of instance to connect to", "<name>");

    auto status = parser->commandParse(this);

    if (status != ParseCode::Ok)
    {
        return status;
    }

    if (parser->positionalArguments().count() == 0)
    {
        cerr << "Name argument is required" << std::endl;
        status = ParseCode::CommandLineError;
    }
    else if (parser->positionalArguments().count() > 1)
    {
        cerr << "Too many arguments given" << std::endl;
        status = ParseCode::CommandLineError;
    }
    else
    {
        request.set_instance_name(parser->positionalArguments().first().toStdString());
    }

    return status;
}
