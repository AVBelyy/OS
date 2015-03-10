all: cat/cat revwords/revwords

cat/cat revwords/revwords:
	$(MAKE) -C $(dir $@) $(notdir $@)
