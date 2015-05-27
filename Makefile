all: cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh

cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh:
	$(MAKE) -C $(dir $@) $(notdir $@)
