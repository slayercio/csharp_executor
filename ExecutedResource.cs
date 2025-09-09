using System;
using System.Threading.Tasks;
using CitizenFX.Core;
using static CitizenFX.Core.Native.API;

namespace ExecutedResource.Client
{
    public class ClientMain : BaseScript
    {
        public ClientMain()
        {
            Debug.WriteLine("Hi from ExecutedResource.Client!");
        }

        [Tick]
        public async Task OnTick()
        {
            if (IsControlJustPressed(0, 167))
            {
                uint model = (uint)GetHashKey("sanchez");

                if (!IsModelInCdimage(model) || !IsModelAVehicle(model))
                {
                    return;
                }

                RequestModel(model);
                while (!HasModelLoaded(model))
                {
                    await Delay(10);
                }

                var ped = PlayerPedId();
                var pos = GetEntityCoords(ped, true);

                float heading = GetEntityHeading(ped);
                float spawnX = pos.X + 2.0f;
                float spawnY = pos.Y + 2.0f;
                float spawnZ = pos.Z;

                int vehicle = CreateVehicle(model, spawnX, spawnY, spawnZ, heading, true, false);

                // Set player into driver seat
                SetPedIntoVehicle(ped, vehicle, -1);

                // Make sure model can be cleaned up
                SetModelAsNoLongerNeeded(model);

                TriggerEvent("chat:addMessage", new
                {
                    color = new[] { 0, 255, 0 },
                    args = new[] { "[Spawner]", "Sanchez spawned!" }
                });
            }
        }
    }
}