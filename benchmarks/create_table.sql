DROP TABLE IF EXISTS vanke;
CREATE TABLE vanke(
        houseID char( 20) NOT NULL PRIMARY KEY,
        housename varchar( 20) NOT NULL UNIQUE,
        buildingNo int NOT NULL,
        unitNo smallint NOT NULL,
        houseprice numeric(15,4) NOT NULL,
        carportid int DEFAULT NULL,
        carportprice numeric(10,3) DEFAULT NULL,
        householdname varchar( 19) NOT NULL,
        moveintime timestamp NOT NULL,
        familysize varchar( 10),
        operatetype  int DEFAULT NULL,
        programcode  varchar( 19) NOT NULL,
        programname  varchar( 19) NOT NULL,
        programnamelen  int NOT NULL,
        programtype  int NOT NULL DEFAULT '0',
        contentcode  varchar( 19)   NOT NULL,
        seriesprogramcode  varchar( 19)   NOT NULL,
        isrecommend  varchar( 19) DEFAULT NULL,
        ishot  varchar( 19) DEFAULT NULL,
        isfirstpage  varchar( 19) DEFAULT NULL,
        catagorycode  varchar( 19)   DEFAULT NULL,
        bocode  varchar( 19)   DEFAULT NULL,
        programsearchkey  varchar( 19)   DEFAULT NULL,
        mediaservices  int NOT NULL DEFAULT '0',
        ratingid  int NOT NULL DEFAULT '0',
        recommendid  int NOT NULL DEFAULT '1',
        sortnum  int NOT NULL DEFAULT '1',
        price  int NOT NULL DEFAULT '0',
        enabledtime  varchar( 19)   NOT NULL,
        disabledtime  varchar( 19)   NOT NULL,
        onlinetime  varchar( 19)   NOT NULL,
        offlinetime  varchar( 19)   NOT NULL,
        updatetime  varchar( 19)   NOT NULL,
        createtime  varchar( 19)   NOT NULL,
        countryname  varchar( 19)   NOT NULL,
        issimpletrailer  int DEFAULT NULL,
        telecomcode  varchar( 19)   DEFAULT NULL,
        mediacode  varchar( 19)   DEFAULT NULL,
        trailerbegintime  varchar( 19)   DEFAULT NULL,
        trailerendtime  varchar( 7)   DEFAULT NULL
);

/*********

CREATE TABLE vanke(
    houseID char(20) PRIMARY KEY, housename varchar(20) NOT NULL UNIQUE,
    buildingNo int NOT NULL, unitNo smallint NOT NULL,
    houseprice numeric(15,4) NOT NULL,
    carportid int, carportprice numeric(10,3),
    householdname varchar(19) NOT NULL,
    moveintime timestamp NOT NULL, familysize varchar(10),
    operatetype int, programcode varchar(19) NOT NULL,
    programname varchar(19) NOT NULL, programnamelen int NOT NULL,
    programtype int DEFAULT 0, contentcode varchar(19) NOT NULL,
    seriesprogramcode varchar(19) NOT NULL,
    isrecommend varchar(19), ishot varchar(19), isfirstpage varchar(19),
    catagorycode varchar(19), bocode varchar(19), programsearchkey varchar(19),
    mediaservices int DEFAULT 0, ratingid int DEFAULT 0,
    recommendid int DEFAULT 1, sortnum int DEFAULT 1, price int DEFAULT 0,
    enabledtime varchar(19) NOT NULL, disabledtime varchar(19) NOT NULL,
    onlinetime varchar(19) NOT NULL, offlinetime varchar(19) NOT NULL,
    updatetime varchar(19) NOT NULL, createtime varchar(19) NOT NULL,
    countryname varchar(19) NOT NULL, issimpletrailer int,
    telecomcode varchar(19), mediacode varchar(19),
    trailerbegintime varchar(19), trailerendtime varchar(7)
);

CREATE TABLE vanke1 (LIKE vanke);
ALTER TABLE vanke1 DROP CONSTRAINT IF EXISTS vanke1_pkey;
ALTER TABLE vanke1 DROP CONSTRAINT IF EXISTS vanke1_housename_key;

**********/