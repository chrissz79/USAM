export const DB_FILE = "usam.db";

export const IPC_CHANNELS = {
  SEARCH: "usam:search",
  GET_PLUGINS: "usam:get-plugins",
  GET_TAGS: "usam:get-tags",
  ASSIGN_TAG: "usam:assign-tag",
  REMOVE_TAG: "usam:remove-tag",
  START_SCAN: "usam:start-scan",
  SCAN_STATUS: "usam:scan-status",
} as const;
