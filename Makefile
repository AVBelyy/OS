all: cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender

cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender:
	$(MAKE) -C $(dir $@) $(notdir $@)
