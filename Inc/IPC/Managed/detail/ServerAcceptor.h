#pragma once

#include "Transport.h"
#include "Server.h"
#include "ResumableBase.h"
#include "NativeObject.h"
#include "ManagedCallback.h"

#include <msclr/marshal.h>


namespace IPC
{
namespace Managed
{
    namespace detail
    {
        template <typename Request, typename Response>
        ref class Transport<Request, Response>::ServerAcceptor : ResumableBase<NativeServerAcceptor>, IServerAcceptor
        {
        public:
            ServerAcceptor(System::String^ name, HandlerFactory^ handlerFactory, const NativeConfig& config)
                : m_name{ name },
                  m_handlerFactory{ handlerFactory },
                  m_config{ config }
            {}

            virtual event System::EventHandler<ComponentEventArgs<IServer^>^>^ Accepted;

            virtual event System::EventHandler<ErrorEventArgs^>^ Error;

            virtual void Start()
            {
                if (Enabled)
                {
                    throw gcnew Exception{ "Acceptor has already stared." };
                }

                Enabled = true;
            }

            virtual void Stop()
            {
                if (!Enabled)
                {
                    throw gcnew Exception{ "Acceptor has already stopped." };
                }

                Enabled = false;
            }

        protected:
            NativeServerAcceptor MakeResumable() override
            {
                return NativeServerAcceptor{
                    msclr::interop::marshal_context().marshal_as<const char*>(m_name),
                    MakeManagedCallback(gcnew AcceptedLambda(this, *m_config)),
                    **m_config };
            }

        internal:
            ref struct AcceptedLambda
            {
                AcceptedLambda(ServerAcceptor^ acceptor, const NativeConfig& config)
                    : m_acceptor{ acceptor },
                      m_config{ config }
                {}

                void operator()(Interop::Callback<typename NativeServer::ConnectionPtr()>&& getConnection)
                {
                    try
                    {
                        Server^ server = nullptr;

                        try
                        {
                            server = gcnew Server{ getConnection(), m_acceptor->m_handlerFactory, *m_config };
                        }
                        catch (const std::exception& /*e*/)
                        {
                            ThrowManagedException(std::current_exception());
                        }

                        m_acceptor->Accepted(m_acceptor, gcnew ComponentEventArgs<IServer^>{ server });
                    }
                    catch (System::Exception^ e)
                    {
                        m_acceptor->Error(m_acceptor, gcnew ErrorEventArgs{ e });
                    }
                }

                ServerAcceptor^ m_acceptor;
                NativeObject<NativeConfig> m_config;
            };

        private:
            System::String^ m_name;
            HandlerFactory^ m_handlerFactory;
            NativeObject<NativeConfig> m_config;
        };

    } // detail

} // Managed
} // IPC
