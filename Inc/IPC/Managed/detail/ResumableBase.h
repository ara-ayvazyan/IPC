#pragma once

#include "NativeObject.h"

#pragma managed(push, off)
#include <boost/optional.hpp>
#pragma managed(pop)


namespace IPC
{
namespace Managed
{
    namespace detail
    {
        template <typename T>
        ref class ResumableBase abstract
        {
        public:
            virtual property System::Boolean Enabled
            {
                System::Boolean get()
                {
                    return static_cast<bool>(*m_resumable);
                }

                void set(System::Boolean value)
                {
                    if (Enabled != value)
                    {
                        try
                        {
                            if (value)
                            {
                                (*m_resumable).emplace(MakeResumable());
                            }
                            else
                            {
                                (*m_resumable).reset();
                            }
                        }
                        catch (const std::exception& /*e*/)
                        {
                            ThrowManagedException(std::current_exception());
                        }
                    }
                }
            }

        protected:
            ResumableBase()
            {}

            virtual T MakeResumable() abstract;

            property T& Resumable
            {
                T& get()
                {
                    if (!Enabled)
                    {
                        throw gcnew Exception{ "Accessing disabled object." };
                    }

                    return **m_resumable;
                }
            }

        private:
            NativeObject<boost::optional<T>> m_resumable;
        };

    } // detail

} // Managed
} // IPC
