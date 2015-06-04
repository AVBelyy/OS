all: cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking bipiper/polling

cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking bipiper/polling:
	$(MAKE) -C $(dir $@) $(notdir $@)
