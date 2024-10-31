#pragma once

void
GSNotifyStackChange(
    IN  PVOID       OldStackBase,
    IN  PVOID       NewStackBase,
    IN  DWORD       StackSize
    );
