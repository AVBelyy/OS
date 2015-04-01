all: cat/cat revwords/revwords filter/filter bufcat/bufcat

cat/cat revwords/revwords filter/filter bufcat/bufcat:
	$(MAKE) -C $(dir $@) $(notdir $@)
