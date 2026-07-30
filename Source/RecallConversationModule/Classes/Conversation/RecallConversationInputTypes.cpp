// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallConversationInputTypes.h"

#include "ConversationTypes.h"
#include "Utility/Input/RecallInputUtils.h"

#define RECALL_CONVERSATION_CHOICE_TYPE_KEY TEXT("ConversationChoiceType")
#define RECALL_CONVERSATION_CHOICE_NODE_KEY TEXT("ConversationChoiceNode")

FRecallConversationInputCommand::FRecallConversationInputCommand(const FString& Options)
{
	Type = Recall::Input::Utils::GetEnumOption(
		Options, RECALL_CONVERSATION_CHOICE_TYPE_KEY, ERecallConversationInputType::None);
	const FString GuidStr = Recall::Input::Utils::GetOption(Options, RECALL_CONVERSATION_CHOICE_NODE_KEY);
	if (!GuidStr.IsEmpty())
	{
		NodeGUID = FGuid(GuidStr);
	}
}

FString FRecallConversationInputCommand::ToOptions() const
{
	FString Options;

	Recall::Input::Utils::AddEnumOption(RECALL_CONVERSATION_CHOICE_TYPE_KEY, Options, Type);
	Recall::Input::Utils::AddOption(RECALL_CONVERSATION_CHOICE_NODE_KEY, Options, NodeGUID.ToString());

	return Options;
}

bool FRecallConversationInputCommand::IsValid() const
{
	return Type != ERecallConversationInputType::None;
}

FConversationChoiceReference FRecallConversationInputCommand::ToChoiceReference() const
{
	return FConversationChoiceReference(NodeGUID);
}
