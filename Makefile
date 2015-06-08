all: cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking bipiper/polling

cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking bipiper/polling:
	$(MAKE) -C $(dir $@) $(notdir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean
	$(MAKE) -C bufcat clean
	$(MAKE) -C simplesh clean
	$(MAKE) -C filesender clean
	$(MAKE) -C bipiper clean
