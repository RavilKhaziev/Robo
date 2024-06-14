#pragma once

namespace RoboDrive{ 
    template <typename StateEnum>
    class IStateMachine{
            public:
                virtual void Update();
                virtual void AddState(IState<StateEnum> *state);
                virtual void NextState(const StateEnum &state);
                virtual bool Start(const StateEnum &state);
    };
}