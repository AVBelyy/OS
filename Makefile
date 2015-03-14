all: cat/cat revwords/revwords filter/filter

cat/cat revwords/revwords filter/filter:
	$(MAKE) -C $(dir $@) $(notdir $@)
