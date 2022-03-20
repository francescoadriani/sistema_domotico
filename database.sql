-- historian.history definition

CREATE TABLE `history` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `data` double NOT NULL,
  `timestamp` datetime NOT NULL,
  `um` varchar(20) DEFAULT NULL,
  `topic` varchar(255) NOT NULL,
  `senderId` varchar(100) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
COLLATE=utf8mb4_general_ci;
CREATE INDEX history_name_IDX USING BTREE ON historian.history (name);
CREATE INDEX history_timestamp_IDX USING BTREE ON historian.history (`timestamp`);
