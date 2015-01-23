ipa
select a.* from (SELECT t.name, t.zone, (select count(*)  from `a_record` t1 where t1.name=t.name and t1.zone=t.zone) as cnt FROM `a_record` t ) a where a.cnt > 1