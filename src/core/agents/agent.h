#ifndef AGENT_H
#define AGENT_H
#include <string>

struct AgentId {
	int id;
};

struct AgentTypeId {
	int id;
};

struct Personality {
	float growth_modifier;		   // Agent’s drive to
								   // expand operations and
								   // acquire new assets.
	float aggression_modifier;	   // Agent’s tendency
								   // to compete
								   // aggressively or
								   // sabotage rivals.
	float preservation_modifier;   // Agent’s caution
								   // level;
								   // prioritizes
								   // safety and risk
								   // avoidance.
	float innovation_modifier;	   // Agent’s
								   // motivation to
								   // pursue new
								   // technologies
								   // and improve
								   // efficiency.
	float collaboration_modifier;  // Agent’s
								   // willingness to
								   // form alliances
								   // and trade
								   // cooperatively.
	float strategy_modifier;	   // Agent’s focus on
								   // long-term
								   // planning versus
								   // short-term gains.
	float adaptability_modifier;   // Agent’s ability
								   // and willingness
								   // to change
								   // tactics when
								   // conditions
								   // shift.
};

struct AgentType {
	AgentTypeId id;
	Personality personality;
	const float frustration_threshold;	// When
										// personality
										// will shift due
										// to actions not
										// being filled.
};

struct Agent {
	AgentId id;
	std::string name;
	float capital;
	float frustration = 0.0f;  // How often an Agent has not been
							   // able to perform a desired
							   // actions.
};

#endif	// AGENT_H
