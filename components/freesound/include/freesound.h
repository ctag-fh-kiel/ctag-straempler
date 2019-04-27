#pragma once

void freesoundInit(xQueueHandle);
void freesoundGetTags(const char *path);
void freesoundSearch(const char *query);
void freesoundGetInstance(const char *id);
void freesoundSetToken(const char *token);