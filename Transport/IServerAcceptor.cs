using System;

namespace IPC.Managed
{
    public interface IServerAcceptor<Request, Response> : IBackgroundError
    {
        event EventHandler<ComponentEventArgs<IServer<Request, Response>>> Accepted;

        bool Enabled { get; set; }

        void Start();

        void Stop();
    }
}
